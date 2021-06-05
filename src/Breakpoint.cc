#include "Breakpoint.h"

#include "Ensure/Ensure.h"

const char overrideInstruction[] = { (char)0xcc };
const size_t overrideInstructionLength = sizeof(overrideInstruction);

Breakpoint::Breakpoint(Process &process, uintptr_t address) : process(process), address(address), activated(false) {}

Breakpoint::~Breakpoint() {
    if (activated) deactivate();
}

void Breakpoint::activate() {
    if (activated) return;
    process.read(address, overrideInstructionLength, originalBytes);
    process.write(address, (void *)overrideInstruction, overrideInstructionLength);
    activated = true;
}

void Breakpoint::deactivate() {
    if (activated == false) return;
    process.write(address, originalBytes);
    activated = false;
}

bool Breakpoint::isStoppedHere() const {
    return process.isStopped() &&
           process.getStoppedSignal() == SIGTRAP &&
           process.getRegister(&Registers::rip) == address + overrideInstructionLength;
}

void Breakpoint::fixProgramCounter() {
    process.addToRegister(&Registers::rip, -(int)overrideInstructionLength);
}
