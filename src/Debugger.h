#ifndef _MENCI_MDB_DEBUGGER_H
#define _MENCI_MDB_DEBUGGER_H

#include <map>
#include <unordered_map>

#include "Process.h"
#include "Breakpoint.h"
#include "SourceFile.h"

class Debugger {
public:
    Process process;

private:
    std::map<int, Breakpoint> breakpoints;
    int breakpointId;
    std::map<int, Breakpoint>::iterator currentBreakpoint;
    std::unordered_map<std::string, SourceFile> sourceFiles;

    std::string currentFilename;
    size_t currentLine;

    std::map<int, Breakpoint>::iterator getCurrentBreakpoint();
    bool skipCurrentBreakpoint();
    void printStopReason(bool checkBreakpoints = false, bool checkSignals = true);
    SourceFile &ensureSourceFile(const std::string &filename);
    void printSourceCode(const std::string &filename, size_t startLine, size_t lines = 1);
    void printDisassemble(uintptr_t address, size_t lines);
    static std::string addressToHex(intptr_t x);
    std::string resolveAddress(uintptr_t address, const std::string &stringAddress = "");
    std::string resolveAddress(const std::string &string);

    void resume();

public:
    Debugger(Process &&process);

    void addBreakpoint(intptr_t address);
    void delBreakpoint(int breakpointId);
    void continueRunning();
    void stepIntoInstruction();
    void stepOverInstruction();
    void listSourceCode(size_t startLine);
    void disassemble(intptr_t address);
};

#endif // _MENCI_MDB_DEBUGGER_H
