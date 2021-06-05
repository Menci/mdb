#include "Disassembler.h"

const size_t MAX_INSTRUCTION_LENGTH = 15;
const size_t MAX_INSTRUCTION_TEXT_LENGTH = 256;

Disassembler::Disassembler(intptr_t baseAddress) : runtimeAddress(baseAddress) {
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
    instructionBuffer.reserve(MAX_INSTRUCTION_LENGTH);
}

std::tuple<Disassembler::Status, std::string, intptr_t, std::vector<unsigned char>> Disassembler::pushByte(char byte) {
    if (instructionBuffer.size() == MAX_INSTRUCTION_LENGTH)
        return std::make_tuple(Disassembler::Status::BadStream, "", 0, std::vector<unsigned char>());

    instructionBuffer.push_back(byte);

    ZydisDecodedInstruction instruction;
    if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, instructionBuffer.data(), instructionBuffer.size(), &instruction))) {
        char buffer[MAX_INSTRUCTION_TEXT_LENGTH];
        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer), runtimeAddress);

        intptr_t instructionAddress = runtimeAddress;
        std::vector<unsigned char> instructionData = instructionBuffer;

        runtimeAddress += instructionBuffer.size();
        instructionBuffer.clear();

        return std::make_tuple(
            Disassembler::Status::Success,
            buffer,
            instructionAddress,
            std::move(instructionData)
        );
    }

    return std::make_tuple(Disassembler::Status::Pending, "", 0, std::vector<unsigned char>());
}
