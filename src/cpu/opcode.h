#pragma once
#include <cstdint>

#include "cpu.h"

struct OpCode {
    uint8_t opcode;
    const char* name;
    uint8_t bytes;
    uint8_t cycles;
    AddressingMode mode;
};

const static OpCode opcodes[0xff] = {
    // INX
    [0xE8] = {0xE8, "INX", 1, 2, NONE_ADDRESSING},

    // LDA
    [0xA9] = {0xA9, "LDA", 2, 2, IMMEDIATE},
    [0xA5] = {0xA5, "LDA", 2, 3, ZEROPAGE},
    [0xB5] = {0xB5, "LDA", 2, 4, ZEROPAGE_X},
    [0xAD] = {0xAD, "LDA", 3, 4, ABSOLUTE},
    [0xBD] = {0xBD, "LDA", 3, 4 /* (+1 if page crossed) */, ABSOLUTE_X},
    [0xB9] = {0xB9, "LDA", 3, 4 /* (+1 if page crossed) */, ABSOLUTE_Y},
    [0xA1] = {0xA1, "LDA", 2, 6, INDIRECT_X},
    [0xB1] = {0xB1, "LDA", 2, 5 /* (+1 if page crossed) */, INDIRECT_Y},

    // LDX
    [0xA2] = {0xA2, "LDX", 2, 2, IMMEDIATE},
    [0xA6] = {0xA6, "LDX", 2, 3, ZEROPAGE},
    [0xB6] = {0xB6, "LDX", 2, 4, ZEROPAGE_Y},
    [0xAE] = {0xAE, "LDX", 3, 4, ABSOLUTE},
    [0xBE] = {0xBE, "LDX", 3, 4 /* (+1 if page crossed) */, ABSOLUTE_Y},

    // STA
    [0x85] = {0x85, "STA", 2, 3, ZEROPAGE},
    [0x95] = {0x95, "STA", 2, 4, ZEROPAGE_X},
    [0x8D] = {0x8D, "STA", 3, 4, ABSOLUTE},
    [0x9D] = {0x9D, "STA", 3, 5, ABSOLUTE_X},
    [0x99] = {0x99, "STA", 3, 5, ABSOLUTE_Y},
    [0x81] = {0x81, "STA", 2, 6, INDIRECT_X},
    [0x91] = {0x91, "STA", 2, 6, INDIRECT_Y},

    // INX
    [0xAA] = {0xAA, "TAX", 1, 2, NONE_ADDRESSING},
};
