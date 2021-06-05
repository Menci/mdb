#ifndef _MENCI_MDB_PROCESS_H
#define _MENCI_MDB_PROCESS_H

#include <vector>
#include <cstdint>
#include <unistd.h>
#include <sys/user.h>
#include <signal.h>

#include "ELF.h"

using Registers = user_regs_struct;
using RegisterData = decltype(Registers().rip);
using RegisterId = RegisterData Registers::*;

class Process {
public:
    ELF elf;
    pid_t pid;
    int status;
    bool isAttached;

    Process(const std::string &filename);
    Process(pid_t pid);
    Process(const Process &) = delete;
    Process(Process &&);
    ~Process();

    int wait();
    bool read(uintptr_t address, void *buffer, size_t length);
    bool read(uintptr_t address, size_t length, std::vector<char> &result);
    std::vector<char> read(uintptr_t address, size_t length);
    void write(uintptr_t address, void *data, size_t length);
    void write(uintptr_t address, std::vector<char> data);

    Registers getRegisters() const;
    RegisterData getRegister(RegisterId registerId) const;
    void setRegisters(const Registers &registers);
    void addToRegister(RegisterId registerId, int64_t delta);
    void setRegister(RegisterId registerId, RegisterData value);

    void resume();
    void stepIntoInstruction();

    bool isStopped() const;
    int getStoppedSignal() const;
    bool isExited() const;
    int getExitCode() const;

private:
    void jobControlEnter() const;
    void jobControlLeave() const;
};

#endif // _MENCI_MDB_PROCESS_H
