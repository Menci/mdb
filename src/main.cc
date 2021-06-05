#include <iostream>
#include <memory>
#include <readline/readline.h>
#include <map>

#include "TerminalColor/TerminalColor.h"

#include "Arguments.h"
#include "Process.h"
#include "REPL.h"
#include "Debugger.h"
#include "Util.h"

const char *PROMPT = "mdb";

int main(int argc, char *argv[]) {
    Arguments arguments = parseArguments(argc, argv);

    Debugger debugger = !arguments.executable.empty()
                      ? Debugger(Process(arguments.executable))
                      : Debugger(Process(arguments.pid));

    Process &process = debugger.process;

    REPL::setTabCompletionList(process.elf.getAllSymbolNames());
    REPL::setPrompt((std::string)PROMPT + " " + std::to_string(process.pid));

    std::unique_ptr<Line> lastLine;
    while (true) {
        bool eof;
        auto thisLine = REPL::readline(eof);
        if (eof) return 0;

        auto line = thisLine.empty && lastLine ? *lastLine : *(lastLine = std::make_unique<Line>(thisLine));
        auto command = line.nextToken();

        if (command == "") continue;

        if (command == "b") {
            auto position = line.nextToken();
            if (position.substr(0, 3) == "*0x") {
                uintptr_t address = std::stoul(position.substr(3), nullptr, 16);
                debugger.addBreakpoint(address);
            } else if (auto symbol = process.elf.lookupSymbol(position))
                debugger.addBreakpoint(symbol.getAddress());
            else {
                std::cout << "Not found in symbol table" << std::endl;
            }
        } else if (command == "d") {
            auto id = std::stoi(line.nextToken());
            debugger.delBreakpoint(id);
        } else if (command == "c") {
            debugger.continueRunning();
        } else if (command == "si") {
            debugger.stepIntoInstruction();
        } else if (command == "ni") {
            debugger.stepOverInstruction();
        } else if (command == "l") {
            auto startLine = line.nextToken();
            debugger.listSourceCode(startLine.empty() ? 0 : std::stoul(startLine));
        } else if (command == "dis") {
            auto addr = line.nextToken();
            if (addr.substr(0, 2) == "0x") {
                uintptr_t address = std::stoul(addr.substr(2), nullptr, 16);
                debugger.disassemble(address);
            } else
                std::cout << "Please enter a 0xXXXXXXXX address" << std::endl;
        } else if (command == "h") {
            std::vector<std::vector<std::string>> help;
            help.push_back({"b <function_name>", "Set a breakpoint on a function"});
            help.push_back({"b *0x<addreess>", "Set a breakpoint on an address"});
            help.push_back({"d <id>", "Delete a breakpoint with id"});
            help.push_back({"c", "Continue running the program"});
            help.push_back({"si", "Step into a single instruction"});
            help.push_back({"ni", "Step over a single instruction"});
            help.push_back({"l", "List source code from current line"});
            help.push_back({"l <line>", "List source code from the specfied line number"});
            help.push_back({"dis", "Disassemble from current PC address"});
            help.push_back({"dis <address>", "Disassemble from the specfied address"});
            help.push_back({"h", "Print this help message"});
            formatTable(help);
            for (const auto &row : help) {
                const std::string &command = row[0];
                const std::string &description = row[1];

                size_t i = command.find_first_of(' ');
                const std::string &commandName = i == std::string::npos ? command : command.substr(0, i);
                const std::string &commandArguments = i == std::string::npos ? "" : command.substr(i + 1);

                std::cout << TerminalColor::Bold
                          << TerminalColor::ForegroundMagenta
                          << commandName
                          << TerminalColor::Reset
                          << TerminalColor::ForegroundMagenta
                          << " " << commandArguments
                          << TerminalColor::Reset
                          << TerminalColor::ForegroundBlue
                          << " " << description
                          << TerminalColor::Reset
                          << std::endl;
            }
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
}
