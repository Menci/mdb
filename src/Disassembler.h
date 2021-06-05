#ifndef _MENCI_MDB_DISASSEMBLER_H
#define _MENCI_MDB_DISASSEMBLER_H

#include <cstdint>
#include <vector>
#include <string>
#include <tuple>

#include <Zydis/Zydis.h>

class Disassembler {
    ZydisDecoder decoder;
    ZydisFormatter formatter;
    intptr_t runtimeAddress;
    std::vector<unsigned char> instructionBuffer;

public:
    enum class Status {
        Success, Pending, BadStream
    };

    Disassembler(intptr_t baseAddress);

    std::tuple<Status, std::string, intptr_t, std::vector<unsigned char>> pushByte(char byte);
};

#endif // _MENCI_MDB_DISASSEMBLER_H
