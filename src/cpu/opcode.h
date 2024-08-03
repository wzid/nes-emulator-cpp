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
    // BRK
    [0X00] = {0X00, "BRK", 1, 7, NONE_ADDRESSING},

    // NOP
    [0XEA] = {0XEA, "NOP", 1, 2, NONE_ADDRESSING},

    // ADC
    [0X69] = {0X69, "ADC", 2, 2, IMMEDIATE},
    [0X65] = {0X65, "ADC", 2, 3, ZEROPAGE},
    [0X75] = {0X75, "ADC", 2, 4, ZEROPAGE_X},
    [0X6D] = {0X6D, "ADC", 3, 4, ABSOLUTE},
    [0X7D] = {0X7D, "ADC", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0X79] = {0X79, "ADC", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0X61] = {0X61, "ADC", 2, 6, INDIRECT_X},
    [0X71] = {0X71, "ADC", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // SBC
    [0XE9] = {0XE9, "SBC", 2, 2, IMMEDIATE},
    [0XE5] = {0XE5, "SBC", 2, 3, ZEROPAGE},
    [0XF5] = {0XF5, "SBC", 2, 4, ZEROPAGE_X},
    [0XED] = {0XED, "SBC", 3, 4, ABSOLUTE},
    [0XFD] = {0XFD, "SBC", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0XF9] = {0XF9, "SBC", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0XE1] = {0XE1, "SBC", 2, 6, INDIRECT_X},
    [0XF1] = {0XF1, "SBC", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // AND
    [0X29] = {0X29, "AND", 2, 2, IMMEDIATE},
    [0X25] = {0X25, "AND", 2, 3, ZEROPAGE},
    [0X35] = {0X35, "AND", 2, 4, ZEROPAGE_X},
    [0X2D] = {0X2D, "AND", 3, 4, ABSOLUTE},
    [0X3D] = {0X3D, "AND", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0X39] = {0X39, "AND", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0X21] = {0X21, "AND", 2, 6, INDIRECT_X},
    [0X31] = {0X31, "AND", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // EOR
    [0X49] = {0X49, "EOR", 2, 2, IMMEDIATE},
    [0X45] = {0X45, "EOR", 2, 3, ZEROPAGE},
    [0X55] = {0X55, "EOR", 2, 4, ZEROPAGE_X},
    [0X4D] = {0X4D, "EOR", 3, 4, ABSOLUTE},
    [0X5D] = {0X5D, "EOR", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0X59] = {0X59, "EOR", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0X41] = {0X41, "EOR", 2, 6, INDIRECT_X},
    [0X51] = {0X51, "EOR", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // ORA
    [0X09] = {0X09, "ORA", 2, 2, IMMEDIATE},
    [0X05] = {0X05, "ORA", 2, 3, ZEROPAGE},
    [0X15] = {0X15, "ORA", 2, 4, ZEROPAGE_X},
    [0X0D] = {0X0D, "ORA", 3, 4, ABSOLUTE},
    [0X1D] = {0X1D, "ORA", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0X19] = {0X19, "ORA", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0X01] = {0X01, "ORA", 2, 6, INDIRECT_X},
    [0X11] = {0X11, "ORA", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // ASL
    [0X0A] = {0X0A, "ASL", 1, 2, NONE_ADDRESSING},
    [0X06] = {0X06, "ASL", 2, 5, ZEROPAGE},
    [0X16] = {0X16, "ASL", 2, 6, ZEROPAGE_X},
    [0X0E] = {0X0E, "ASL", 3, 6, ABSOLUTE},
    [0X1E] = {0X1E, "ASL", 3, 7, ABSOLUTE_X},

    // LSR
    [0X4A] = {0X4A, "LSR", 1, 2, NONE_ADDRESSING},
    [0X46] = {0X46, "LSR", 2, 5, ZEROPAGE},
    [0X56] = {0X56, "LSR", 2, 6, ZEROPAGE_X},
    [0X4E] = {0X4E, "LSR", 3, 6, ABSOLUTE},
    [0X5E] = {0X5E, "LSR", 3, 7, ABSOLUTE_X},

    // ROL
    [0X2A] = {0X2A, "ROL", 1, 2, NONE_ADDRESSING},
    [0X26] = {0X26, "ROL", 2, 5, ZEROPAGE},
    [0X36] = {0X36, "ROL", 2, 6, ZEROPAGE_X},
    [0X2E] = {0X2E, "ROL", 3, 6, ABSOLUTE},
    [0X3E] = {0X3E, "ROL", 3, 7, ABSOLUTE_X},

    // ROR
    [0X6A] = {0X6A, "ROR", 1, 2, NONE_ADDRESSING},
    [0X66] = {0X66, "ROR", 2, 5, ZEROPAGE},
    [0X76] = {0X76, "ROR", 2, 6, ZEROPAGE_X},
    [0X6E] = {0X6E, "ROR", 3, 6, ABSOLUTE},
    [0X7E] = {0X7E, "ROR", 3, 7, ABSOLUTE_X},

    // INC
    [0XE6] = {0XE6, "INC", 2, 5, ZEROPAGE},
    [0XF6] = {0XF6, "INC", 2, 6, ZEROPAGE_X},
    [0XEE] = {0XEE, "INC", 3, 6, ABSOLUTE},
    [0XFE] = {0XFE, "INC", 3, 7, ABSOLUTE_X},

    // INX
    [0XE8] = {0XE8, "INX", 1, 2, NONE_ADDRESSING},

    // INY
    [0XC8] = {0XC8, "INY", 1, 2, NONE_ADDRESSING},

    // DEC
    [0XC6] = {0XC6, "DEC", 2, 5, ZEROPAGE},
    [0XD6] = {0XD6, "DEC", 2, 6, ZEROPAGE_X},
    [0XCE] = {0XCE, "DEC", 3, 6, ABSOLUTE},
    [0XDE] = {0XDE, "DEC", 3, 7, ABSOLUTE_X},

    // DEX
    [0XCA] = {0XCA, "DEX", 1, 2, NONE_ADDRESSING},

    // DEY
    [0X88] = {0X88, "DEY", 1, 2, NONE_ADDRESSING},

    // CMP
    [0XC9] = {0XC9, "CMP", 2, 2, IMMEDIATE},
    [0XC5] = {0XC5, "CMP", 2, 3, ZEROPAGE},
    [0XD5] = {0XD5, "CMP", 2, 4, ZEROPAGE_X},
    [0XCD] = {0XCD, "CMP", 3, 4, ABSOLUTE},
    [0XDD] = {0XDD, "CMP", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0XD9] = {0XD9, "CMP", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0XC1] = {0XC1, "CMP", 2, 6, INDIRECT_X},
    [0XD1] = {0XD1, "CMP", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // CPY
    [0XC0] = {0XC0, "CPY", 2, 2, IMMEDIATE},
    [0XC4] = {0XC4, "CPY", 2, 3, ZEROPAGE},
    [0XCC] = {0XCC, "CPY", 3, 4, ABSOLUTE},

    // CPX
    [0XE0] = {0XE0, "CPX", 2, 2, IMMEDIATE},
    [0XE4] = {0XE4, "CPX", 2, 3, ZEROPAGE},
    [0XEC] = {0XEC, "CPX", 3, 4, ABSOLUTE},

    // JMP
    [0X4C] = {0X4C, "JMP", 3, 3, NONE_ADDRESSING /* AddressingMode that acts as Immidiate */},
    [0X6C] = {0X6C, "JMP", 3, 5, NONE_ADDRESSING /* AddressingMode:Indirect with 6502 bug */},

    // JSR
    [0X20] = {0X20, "JSR", 3, 6, NONE_ADDRESSING},

    // RTS
    [0X60] = {0X60, "RTS", 1, 6, NONE_ADDRESSING},

    // RTI
    [0X40] = {0X40, "RTI", 1, 6, NONE_ADDRESSING},

    // BNE
    [0XD0] = {0XD0, "BNE", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BVS
    [0X70] = {0X70, "BVS", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BVC
    [0X50] = {0X50, "BVC", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BMI
    [0X30] = {0X30, "BMI", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BEQ
    [0XF0] = {0XF0, "BEQ", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BCS
    [0XB0] = {0XB0, "BCS", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BCC
    [0X90] = {0X90, "BCC", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BPL
    [0X10] = {0X10, "BPL", 2, 2 /*(+1 if branch succeeds +2 if to a new page)*/, NONE_ADDRESSING},

    // BIT
    [0X24] = {0X24, "BIT", 2, 3, ZEROPAGE},
    [0X2C] = {0X2C, "BIT", 3, 4, ABSOLUTE},

    // LDA
    [0XA9] = {0XA9, "LDA", 2, 2, IMMEDIATE},
    [0XA5] = {0XA5, "LDA", 2, 3, ZEROPAGE},
    [0XB5] = {0XB5, "LDA", 2, 4, ZEROPAGE_X},
    [0XAD] = {0XAD, "LDA", 3, 4, ABSOLUTE},
    [0XBD] = {0XBD, "LDA", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},
    [0XB9] = {0XB9, "LDA", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},
    [0XA1] = {0XA1, "LDA", 2, 6, INDIRECT_X},
    [0XB1] = {0XB1, "LDA", 2, 5/*+1 if page crossed*/, INDIRECT_Y},

    // LDX
    [0XA2] = {0XA2, "LDX", 2, 2, IMMEDIATE},
    [0XA6] = {0XA6, "LDX", 2, 3, ZEROPAGE},
    [0XB6] = {0XB6, "LDX", 2, 4, ZEROPAGE_Y},
    [0XAE] = {0XAE, "LDX", 3, 4, ABSOLUTE},
    [0XBE] = {0XBE, "LDX", 3, 4/*+1 if page crossed*/, ABSOLUTE_Y},

    // LDY
    [0XA0] = {0XA0, "LDY", 2, 2, IMMEDIATE},
    [0XA4] = {0XA4, "LDY", 2, 3, ZEROPAGE},
    [0XB4] = {0XB4, "LDY", 2, 4, ZEROPAGE_X},
    [0XAC] = {0XAC, "LDY", 3, 4, ABSOLUTE},
    [0XBC] = {0XBC, "LDY", 3, 4/*+1 if page crossed*/, ABSOLUTE_X},

    // STA
    [0X85] = {0X85, "STA", 2, 3, ZEROPAGE},
    [0X95] = {0X95, "STA", 2, 4, ZEROPAGE_X},
    [0X8D] = {0X8D, "STA", 3, 4, ABSOLUTE},
    [0X9D] = {0X9D, "STA", 3, 5, ABSOLUTE_X},
    [0X99] = {0X99, "STA", 3, 5, ABSOLUTE_Y},
    [0X81] = {0X81, "STA", 2, 6, INDIRECT_X},
    [0X91] = {0X91, "STA", 2, 6, INDIRECT_Y},

    // STX
    [0X86] = {0X86, "STX", 2, 3, ZEROPAGE},
    [0X96] = {0X96, "STX", 2, 4, ZEROPAGE_Y},
    [0X8E] = {0X8E, "STX", 3, 4, ABSOLUTE},

    // STY
    [0X84] = {0X84, "STY", 2, 3, ZEROPAGE},
    [0X94] = {0X94, "STY", 2, 4, ZEROPAGE_X},
    [0X8C] = {0X8C, "STY", 3, 4, ABSOLUTE},

    // CLD
    [0XD8] = {0XD8, "CLD", 1, 2, NONE_ADDRESSING},

    // CLI
    [0X58] = {0X58, "CLI", 1, 2, NONE_ADDRESSING},

    // CLV
    [0XB8] = {0XB8, "CLV", 1, 2, NONE_ADDRESSING},

    // CLC
    [0X18] = {0X18, "CLC", 1, 2, NONE_ADDRESSING},

    // SEC
    [0X38] = {0X38, "SEC", 1, 2, NONE_ADDRESSING},

    // SEI
    [0X78] = {0X78, "SEI", 1, 2, NONE_ADDRESSING},

    // SED
    [0XF8] = {0XF8, "SED", 1, 2, NONE_ADDRESSING},

    // TAX
    [0XAA] = {0XAA, "TAX", 1, 2, NONE_ADDRESSING},

    // TAY
    [0XA8] = {0XA8, "TAY", 1, 2, NONE_ADDRESSING},

    // TSX
    [0XBA] = {0XBA, "TSX", 1, 2, NONE_ADDRESSING},

    // TXA
    [0X8A] = {0X8A, "TXA", 1, 2, NONE_ADDRESSING},

    // TXS
    [0X9A] = {0X9A, "TXS", 1, 2, NONE_ADDRESSING},

    // TYA
    [0X98] = {0X98, "TYA", 1, 2, NONE_ADDRESSING},

    // PHA
    [0X48] = {0X48, "PHA", 1, 3, NONE_ADDRESSING},

    // PLA
    [0X68] = {0X68, "PLA", 1, 4, NONE_ADDRESSING},

    // PHP
    [0X08] = {0X08, "PHP", 1, 3, NONE_ADDRESSING},

    // PLP
    [0X28] = {0X28, "PLP", 1, 4, NONE_ADDRESSING}

};
