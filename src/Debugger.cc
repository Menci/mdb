#include "Debugger.h"

#include <iostream>
#include <sstream>

#include "Highlight.h"
#include "Disassembler.h"
#include "Util.h"

#include "TerminalColor/TerminalColor.h"
#include "RegexReplace/RegexReplace.h"
#include "Ensure/Ensure.h"

const size_t DEFAULT_DISASSEMBLE_LINES = 3;

Debugger::Debugger(Process &&process)
: process(std::move(process)), breakpointId(0), currentBreakpoint(breakpoints.end()) {}

std::map<int, Breakpoint>::iterator Debugger::getCurrentBreakpoint() {
    for (auto it = breakpoints.begin(); it != breakpoints.end(); it++)
        if (it->second.isStoppedHere()) return (currentBreakpoint = it);
    return breakpoints.end();
}

void Debugger::printStopReason(bool checkBreakpoints, bool checkSignals) {
    if (process.isExited()) {
        std::cout << "Process "
                  << TerminalColor::ForegroundBlue
                  << process.pid
                  << TerminalColor::Reset
                  << " exited with code "
                  << TerminalColor::Bold
                  << process.getExitCode()
                  << TerminalColor::Reset
                  << std::endl;
        exit(EXIT_SUCCESS);
    } else if (process.isStopped()) {
        bool isStoppedByBreakpoint = false;
        if (checkBreakpoints)
            if (auto it = getCurrentBreakpoint(); it != breakpoints.end()) {
                isStoppedByBreakpoint = true;
                auto &[id, breakpoint] = *it;
                breakpoint.fixProgramCounter();
                std::cout << "\nBreakpoint "
                          << id
                          << ", "
                          << TerminalColor::ForegroundBlue;
            }

        if (!isStoppedByBreakpoint && checkSignals) {
            int signal = process.getStoppedSignal();
            std::string signalName = (std::string)"SIG" + sigabbrev_np(signal);
            std::string signalReason = sigdescr_np(signal);
            std::cout << "\nProgram received signal " << signalName << ", " << signalReason << "." << std::endl;
        }
    }

    auto pc = process.getRegister(&Registers::rip);
    auto symbol = process.elf.lookupSymbol(pc);
    std::cout << resolveAddress(pc)
              << " in "
              << TerminalColor::ForegroundYellow
              << (symbol ? symbol.getName() : "???")
              << TerminalColor::Reset
              << " ()";
    
    if (process.elf.addressToLine(pc, currentFilename, currentLine)) {
        std::cout << " at " << TerminalColor::ForegroundGreen << currentFilename << TerminalColor::Reset << ":" << currentLine << std::endl;
        printSourceCode(currentFilename, currentLine);
    } else
        std::cout << std::endl;
    
    printDisassemble(pc, DEFAULT_DISASSEMBLE_LINES);
}

SourceFile &Debugger::ensureSourceFile(const std::string &filename) {
    if (auto it = sourceFiles.find(filename); it != sourceFiles.end()) {
        return it->second;
    }
    return sourceFiles.emplace(filename, filename).first->second;
}

void Debugger::printSourceCode(const std::string &filename, size_t startLine, size_t lines) {
    auto &sourceFile = ensureSourceFile(filename);
    std::stringstream ss;
    for (size_t i = startLine; i != startLine + lines; i++) {
        if (!sourceFile.hasLine(i)) {
            lines = i - startLine;
            break;
        }
        ss << sourceFile.getLine(i) << std::endl;
    }

    std::stringstream highlighted(Highlight::highlight(ss.str(), HighlightLanguage::Cpp));

    for (size_t i = startLine; i != startLine + lines; i++) {
        std::string line;
        std::getline(highlighted, line);
        std::cout << i << "\t\t" << line << TerminalColor::Reset << std::endl;
    }
}

std::string Debugger::addressToHex(intptr_t x) {
    std::stringstream ss;
    ss << TerminalColor::ForegroundBlue << std::hex << "0x" << x << TerminalColor::Reset;
    return ss.str();
}

std::string Debugger::resolveAddress(uintptr_t address, const std::string &stringAddress) {
    const std::string &str = stringAddress.empty() ? addressToHex(address) : stringAddress;
    if (auto symbol = process.elf.lookupSymbol(address)) {
        std::stringstream ss;
        ss << str << TerminalColor::Reset << " <" << TerminalColor::ForegroundYellow << symbol.getName() << TerminalColor::Reset;
        if (symbol.getAddress() != address)
            ss << "+" << address - symbol.getAddress();
        ss << ">";

        return ss.str();
    }
    return str;
}

std::string Debugger::resolveAddress(const std::string &string) {
    static std::regex regex("0x[0-9a-zA-Z]{3,}");
    return regexReplace(string, regex, [&] (const std::smatch &match) {
        std::string str = match.str();
        uintptr_t address = std::stoul(str.substr(2), nullptr, 16);
        return resolveAddress(address, str);
    });
}

void Debugger::printDisassemble(uintptr_t address, size_t lines) {
    std::vector<std::vector<std::string>> disassemble;
    Disassembler disassembler(address);
    while (disassemble.size() < lines) {
        char byte;
        if (!process.read(address++, &byte, 1)) break;
        
        auto [status, instructionText, instructionAddress, instructionData] = disassembler.pushByte(byte);

        if (status == Disassembler::Status::Pending) continue;
        else if (status == Disassembler::Status::BadStream) break;
        else {
            std::vector<std::string> line;

            std::stringstream streamAddress;
            streamAddress << TerminalColor::ForegroundBlue << "0x" << std::hex << instructionAddress << std::dec << TerminalColor::Reset;

            std::stringstream streamData;
            for (size_t i = 0; i < instructionData.size(); i++) {
                unsigned char byte = instructionData[i];
                streamData << " "[i == 0] << std::hex << byte / 16 << byte % 16;
            }

            line.push_back((disassemble.size() == 0 ? "=> " : "   ") + resolveAddress(streamAddress.str()));
            line.push_back(streamData.str());
            line.push_back(resolveAddress(Highlight::highlight(instructionText, HighlightLanguage::ASM)));
            disassemble.push_back(line);
        }
    }

    formatTable(disassemble, {16, 24, 0});

    for (const auto &row : disassemble)
        std::cout << row[0] << " " << row[1] << " " << row[2] << std::endl;
}

void Debugger::resume() {
    for (auto it = breakpoints.begin(); it != breakpoints.end(); it++)
        if (it != currentBreakpoint)
            it->second.activate();
    currentBreakpoint = breakpoints.end();

    process.resume();
    for (auto it = breakpoints.rbegin(); it != breakpoints.rend(); it++)
        it->second.deactivate();
}

void Debugger::addBreakpoint(intptr_t address) {
    int id = ++breakpointId;
    auto breakpoint = Breakpoint(process, address);
    breakpoints.emplace(id, std::move(breakpoint));
    std::cout << "Breakpoint " << id << " at " << resolveAddress(address);

    std::string filename;
    size_t line;
    if (process.elf.addressToLine(address, filename, line)) {
        std::cout << ": file " << TerminalColor::ForegroundGreen << filename << TerminalColor::Reset << ", line " << line << ".";
    }
    std::cout << std::endl;
}

void Debugger::delBreakpoint(int breakpointId) {
    auto it = breakpoints.find(breakpointId);
    if (it == breakpoints.end()) {
        std::cout << "Breakpoint id not found" << std::endl;;
    } else {
        breakpoints.erase(it);
    }
}

void Debugger::continueRunning() {
    std::cout << "Continuing." << std::endl;
    
    resume();

    printStopReason(true);
}

void Debugger::stepIntoInstruction() {
    process.stepIntoInstruction();
    printStopReason(false, false);
}

void Debugger::stepOverInstruction() {
    uintptr_t address = process.getRegister(&Registers::rip), nextAddress = 0;
    Disassembler disassembler(address);
    while (true) {
        char byte;
        if (!process.read(address++, &byte, 1)) break;
        
        auto [status, instructionText, instructionAddress, instructionData] = disassembler.pushByte(byte);
        if (status == Disassembler::Status::BadStream) break;
        else if (status == Disassembler::Status::Success) {
            nextAddress = instructionAddress + instructionData.size();
            break;
        }
    }

    Breakpoint breakpoint(process, nextAddress);
    breakpoint.activate();
    resume();
    breakpoint.deactivate();

    bool stoppedEarlier = !breakpoint.isStoppedHere();
    if (!stoppedEarlier) breakpoint.fixProgramCounter();

    printStopReason(stoppedEarlier, stoppedEarlier);
}

void Debugger::listSourceCode(size_t startLine) {
    if (!startLine) startLine = currentLine;

    if (currentFilename.empty() || currentLine == 0) {
        std::cout << "No source file loaded." << std::endl;
    } else {
        printSourceCode(currentFilename, startLine, 10);
    }
}

void Debugger::disassemble(intptr_t address) {
    printDisassemble(address, 10);
}
