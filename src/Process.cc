#include "Process.h"

#include <sys/uio.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <iostream>
#include <algorithm>

#include "Ensure/Ensure.h"

pid_t selfPid = getpid();

Process::Process(const std::string &filename): elf(filename), isAttached(false) {
    if (pid = fork(); !pid) {
        // Child process
        pid = getpid();
        ENSURE_ERRNO(setpgid(pid, pid));
        ENSURE_ERRNO(ptrace(PTRACE_TRACEME, 0, NULL, NULL));

        char *argv[] = { const_cast<char *>(filename.data()), nullptr };
        ENSURE_ERRNO(execv(filename.data(), argv));
    }

    int status = wait();
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        ERROR("The child process exited unexpectedly");
    }
}

Process::Process(pid_t pid): elf("/proc/" + std::to_string(pid) + "/exe"), pid(pid), isAttached(true) {
    ENSURE_ERRNO(ptrace(PTRACE_ATTACH, pid, NULL, NULL));
}

Process::Process(Process &&other) : elf(std::move(other.elf)) {
    pid = other.pid;
    status = other.status;
    isAttached = other.isAttached;

    other.pid = 0;
}

Process::~Process() {
    if (pid && !isAttached)
        kill(pid, SIGKILL);
}

int Process::wait() {
    waitpid(pid, &status, 0);
    return status;
}

bool Process::read(uintptr_t address, void *buffer, size_t length) {
    iovec local;
    local.iov_base = buffer;
    local.iov_len = length;

    iovec remote;
    remote.iov_base = (void *)address;
    remote.iov_len = length;

    return process_vm_readv(pid, &local, 1, &remote, 1, 0) != -1;
}

bool Process::read(uintptr_t address, size_t length, std::vector<char> &result) {
    result.resize(length);
    return read(address, (void *)result.data(), length);
}

std::vector<char> Process::read(uintptr_t address, size_t length) {
    std::vector<char> result(length);
    ENSURE_ERRNO(read(address, (void *)result.data(), length));
    return result;
}

void Process::write(uintptr_t address, void *data, size_t length) {
    bool aligned = address % sizeof(long) == 0 && length % sizeof(long) == 0;
    uintptr_t alignedAddress = address - address % sizeof(long);
    size_t temp = length + (address - alignedAddress);
    size_t alignedLength = temp % sizeof(long) == 0 ? temp : temp - temp % sizeof(long) + sizeof(long);

    char *buffer = (char *)(aligned ? data : malloc(alignedLength));

    if (!aligned) {
        ENSURE_ERRNO(*(long *)buffer = ptrace(PTRACE_PEEKDATA, pid, alignedAddress, 0));
        ENSURE_ERRNO(*(long *)(buffer + alignedLength - 8) = ptrace(PTRACE_PEEKDATA, pid, alignedAddress + alignedLength - 8, 0));
        memcpy(buffer + (address - alignedAddress), data, length);
    }

    for (size_t i = 0; i < alignedLength; i += sizeof(long))
        ENSURE_ERRNO(ptrace(PTRACE_POKEDATA, pid, alignedAddress + i, *(long *)(buffer + i)));
}

void Process::write(uintptr_t address, std::vector<char> data) {
    write(address, (void *)data.data(), data.size());
}

Registers Process::getRegisters() const {
    Registers registers;
    ENSURE_ERRNO(ptrace(PTRACE_GETREGS, pid, NULL, &registers));
    return registers;
}

RegisterData Process::getRegister(RegisterId registerId) const {
    return getRegisters().*registerId;
}

void Process::setRegisters(const Registers &registers) {
    ENSURE_ERRNO(ptrace(PTRACE_SETREGS, pid, NULL, &registers));
}

void Process::addToRegister(RegisterData Registers::*registerId, int64_t delta) {
    auto registers = getRegisters();
    registers.*registerId += delta;
    setRegisters(registers);
}

void Process::setRegister(RegisterData Registers::*registerId, RegisterData value) {
    auto registers = getRegisters();
    registers.*registerId = value;
    setRegisters(registers);
}

void Process::resume() {
    jobControlEnter();
    ENSURE_ERRNO(ptrace(PTRACE_CONT, pid, NULL, NULL));
    wait();
    jobControlLeave();

    if (isExited()) exit(getExitCode());
}

void Process::stepIntoInstruction() {
    jobControlEnter();
    ENSURE_ERRNO(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL));
    wait();
    jobControlLeave();

    if (isExited()) exit(getExitCode());
}

bool Process::isStopped() const {
    return WIFSTOPPED(status);
}

int Process::getStoppedSignal() const {
    return WSTOPSIG(status);
}

bool Process::isExited() const {
    return WIFEXITED(status);
}

int Process::getExitCode() const {
    return WEXITSTATUS(status);
}

void Process::jobControlEnter() const {
    ENSURE_ERRNO(signal(SIGTTOU, SIG_IGN));
    ENSURE_ERRNO(tcsetpgrp(STDIN_FILENO, pid));
}

void Process::jobControlLeave() const {
    ENSURE_ERRNO(tcsetpgrp(STDIN_FILENO, selfPid));
    ENSURE_ERRNO(signal(SIGTTOU, SIG_DFL));
}
