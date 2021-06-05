#ifndef _MENCI_MDB_BREAKPOINT_H
#define _MENCI_MDB_BREAKPOINT_H

#include "Process.h"

class Breakpoint {
    Process &process;
    uintptr_t address;
    std::vector<char> originalBytes;
    bool activated;

public:
    Breakpoint(Process &process, uintptr_t address);
    Breakpoint(const Breakpoint &) = delete;
    Breakpoint(Breakpoint &&) = default;
    ~Breakpoint();

    void activate();
    void deactivate();
    bool isStoppedHere() const;
    void fixProgramCounter();
};

#endif // _MENCI_MDB_BREAKPOINT_H
