#include "cpu.h"

#include <format>
#include <stdexcept>
#include <vector>

#include "opcode.h"

CPU::CPU() {
    register_a = 0;
    register_x = 0;
    register_y = 0;
    status = 0;
    program_counter = 0;
}

// Program ROM starts at 0x8000 to 0xffff
void CPU::load(std::vector<uint8_t> &program) {
    uint16_t index = 0x8000;
    for (uint8_t opcode : program) {
        memory[index] = opcode;
        index++;
    }
    mem_write_u16(0xfffc, 0x8000);
}
/* NES platform has a special mechanism to mark where the CPU should start the
execution. Upon inserting a new cartridge, the CPU receives a special signal
called "Reset interrupt" that instructs CPU to:

    - reset the state (registers and flags)
    - set program_counter to the 16-bit address that is stored at 0xFFFC
*/
void CPU::reset() {
    register_a = 0;
    register_x = 0;

    // why is 3rd bit to right set?
    status = 0b00100100;

    program_counter = mem_read_u16(0xfffc);
}

void CPU::load_and_run(std::vector<uint8_t> &program) {
    load(program);
    reset();
    run();
}

bool CPU::has_status_flag(uint8_t status_flag) {
    return (status & status_flag) != 0;
}

void CPU::set_status_flag(uint8_t status_flag) { status |= status_flag; }

void CPU::clear_status_flag(uint8_t status_flag) { status &= ~status_flag; }

void CPU::set_status_flag_bit(uint8_t status_flag, bool check) {
    if (check) {
        set_status_flag(status_flag);
    } else {
        clear_status_flag(status_flag);
    }
}

void CPU::add_to_register_a(uint8_t value) {
    // we convert to u16 so that we can tell if there is a carry or and overflow
    uint16_t sum = ((uint16_t)register_a) + value;

    if (has_status_flag(CARRY_FLAG)) {
        sum += 1;
    }

    // if we did not carry
    // otherwise leave the flag in its place
    if (sum <= 0xFF) {
        clear_status_flag(CARRY_FLAG);
    }

    uint8_t result = sum;

    // There are two cases for the overflow flag to appear
    //      1. add two positive numbers and result is negative
    //      2. add two negative numbers and result is positive

    // from https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // We can derive this equation:
    // A = Accumulator before the addition
    // v = the value adding to the accumulator
    // r = the result of the addition (A+v+C)
    // (A ^ r) & (v ^ r) & 0x80

    // this checks if the sign of both inputs is different from the sign of the
    // result
    if ((register_a ^ result) & (value ^ result) & 0x80 != 0) {
        set_status_flag(OVERFLOW_FLAG);
    } else {
        clear_status_flag(OVERFLOW_FLAG);
    }

    set_register_a(result);
}

void CPU::op_ADC(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);
    add_to_register_a(value);
}

void CPU::op_AND(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);
    set_register_a(register_a & value);
}

void CPU::op_ASL_register_a() {
    if (register_a >> 7 == 1) {
        set_status_flag(CARRY_FLAG);
    } else {
        clear_status_flag(CARRY_FLAG);
    }

    set_register_a(register_a >> 1);
}

void CPU::op_ASL(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);

    if (value >> 7 == 1) {
        set_status_flag(CARRY_FLAG);
    } else {
        clear_status_flag(CARRY_FLAG);
    }

    value >>= 1;
    mem_write(addr, value);
    update_zero_and_negative_flags(value);
}

void CPU::branch() {
    uint8_t jump = mem_read(program_counter);
    // since the location is relative to the branch we have to increment by
    // the program counter and then 1 to start at the next instruction
    uint8_t jump_addr = program_counter + jump + 1;

    program_counter = jump_addr;
}

void CPU::op_BIT(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);

    if (register_a & value == 0) {
        set_status_flag(ZERO_FLAG);
    } else {
        clear_status_flag(ZERO_FLAG);
    }

    //  Bits 7 and 6 of the value from memory are copied into the N and V flags.

    set_status_flag_bit(NEGATIVE_FLAG, value & (1 << 7));
    set_status_flag_bit(OVERFLOW_FLAG, value & (1 << 6));
}

void CPU::op_CLC() { clear_status_flag(CARRY_FLAG); }

void CPU::op_INX() {
    register_x++;
    update_zero_and_negative_flags(register_x);
}

void CPU::op_LDA(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);
    set_register_a(value);
}

void CPU::op_LDX(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);
    set_register_x(value);
}

void CPU::op_STA(AddressingMode &mode) {
    auto addr = get_operand_address(mode);
    mem_write(addr, register_a);
}

void CPU::op_TAX() { set_register_x(register_a); }

void CPU::update_zero_and_negative_flags(uint8_t register_to_check) {
    // This sets the zero flag based on if the result of the
    // last exectuted instruction was zero or not.
    if (register_to_check == 0) {
        set_status_flag(ZERO_FLAG);
    } else {
        clear_status_flag(ZERO_FLAG);
    }

    // This sets the negative flag based on if the 7th bit of the
    // last exectuted instruction was set or not.
    if ((register_to_check & NEGATIVE_FLAG) != 0) {
        set_status_flag(NEGATIVE_FLAG);
    } else {
        clear_status_flag(NEGATIVE_FLAG);
    }
}

// We use a uint16 because memory has a length greater than uint8_t
// We have to utilize the larger 16 bit unsigned integer to locate the value in
// memory to read
uint8_t CPU::mem_read(uint16_t address) { return memory[address]; }

void CPU::mem_write(uint16_t address, uint8_t data) { memory[address] = data; }

// NES is written with little endian
// LDA $8000      <=>    ad 00 80
// We need to be able to interpret these in our program correctly
// In this function we read the little endian and convert it to big endian
uint16_t CPU::mem_read_u16(uint16_t pos) {
    uint16_t lower = mem_read(pos);
    uint16_t higher = mem_read(pos + 1);
    // move the higher up 8 bits and then add the lower using OR operation
    return (higher << 8) | lower;
}

// A lot of this is needed to just convert one u16 to two u8s
// then we put lower first and higher last since NES uses little endian
void CPU::mem_write_u16(uint16_t pos, uint16_t data) {
    // move higher bits back down by 8 bits to work in u8
    uint8_t higher = (data >> 8);
    // We use an AND operation to preserve the first 8 bits (0xff)
    // Since 0xff is 8 bits we end up reseting the higher bits to 0
    // becuase in 0xff they are 0/not set
    uint8_t lower = (data & 0xff);

    mem_write(pos, lower);
    mem_write(pos + 1, higher);
}

uint16_t CPU::get_operand_address(AddressingMode &mode) {
    switch (mode) {
        case IMMEDIATE:
            return program_counter;
        case ZEROPAGE:
            return mem_read(program_counter);
        case ABSOLUTE:
            return mem_read_u16(program_counter);
        case ZEROPAGE_X: {
            uint8_t pos = mem_read(program_counter);
            return pos + register_x;
        }
        case ZEROPAGE_Y: {
            uint8_t pos = mem_read(program_counter);
            return pos + register_y;
        }
        case ABSOLUTE_X: {
            uint16_t pos = mem_read_u16(program_counter);
            return pos + register_x;
        }
        case ABSOLUTE_Y: {
            uint16_t pos = mem_read_u16(program_counter);
            return pos + register_y;
        }
        case INDIRECT_X: {
            // https://skilldrick.github.io/easy6502/#indexed-indirect-c0x
            uint8_t pos = mem_read(program_counter);
            uint8_t addr = pos + register_x;
            uint16_t lower = mem_read(addr);
            uint16_t higher = mem_read(addr + 1);
            return (higher << 8) | lower;
        }
        case INDIRECT_Y: {
            // https://skilldrick.github.io/easy6502/#indirect-indexed-c0y
            uint8_t pos = mem_read(program_counter);
            uint16_t lower = mem_read(pos);
            uint16_t higher = mem_read(pos + 1);
            uint16_t final_addr = (higher << 8) | lower;
            return final_addr + register_y;
        }
        case NONE_ADDRESSING: {
            const char *mode_name = addressing_mode_name[1];
            throw std::invalid_argument(
                std::format("Mode {} is not supported", mode_name));
        }
    }
}

void CPU::run() {
    while (1) {
        uint8_t hex_code = mem_read(program_counter);
        program_counter++;
        uint16_t old_program_counter = program_counter;

        OpCode opcode = opcodes[hex_code];

        switch (hex_code) {
            // ADC
            case 0x69:
            case 0x65:
            case 0x75:
            case 0x6D:
            case 0x7D:
            case 0x79:
            case 0x61:
            case 0x71:
                op_ADC(opcode.mode);
                break;

            // AND
            case 0x29:
            case 0x25:
            case 0x35:
            case 0x2D:
            case 0x3D:
            case 0x39:
            case 0x21:
            case 0x31:
                op_AND(opcode.mode);
                break;

            // ASL Accumulator
            case 0x0A:
                op_ASL_register_a();
                break;
            // ASL
            case 0x06:
            case 0x16:
            case 0x0E:
            case 0x1E:
                op_ASL(opcode.mode);
                break;

            // BCC
            case 0x90:
                if (!has_status_flag(CARRY_FLAG)) {
                    branch();
                }
                break;

            // BCS
            case 0xB0:
                if (has_status_flag(CARRY_FLAG)) {
                    branch();
                }
                break;

            // BEQ
            case 0xF0:
                if (has_status_flag(ZERO_FLAG)) {
                    branch();
                }
                break;

            // BIT
            case 0x24:
            case 0x2C:
                op_BIT(opcode.mode);
                break;

            // BMI
            case 0x30:
                if (has_status_flag(NEGATIVE_FLAG)) {
                    branch();
                }
                break;

            // BNE
            case 0xD0:
                if (!has_status_flag(ZERO_FLAG)) {
                    branch();
                }
                break;

            // BPL
            case 0x10:
                if (!has_status_flag(NEGATIVE_FLAG)) {
                    branch();
                }
                break;

            // BVC
            case 0x50:
                if (!has_status_flag(OVERFLOW_FLAG)) {
                    branch();
                }
                break;

            // BVS
            case 0x70:
                if (has_status_flag(OVERFLOW_FLAG)) {
                    branch();
                }
                break;

            // CLC
            case 0x18:
                op_CLC();
                break;

            case 0xE8:
                op_INX();
                break;

            case 0xA9:
            case 0xA5:
            case 0xB5:
            case 0xAD:
            case 0xBD:
            case 0xB9:
            case 0xA1:
            case 0xB1:
                op_LDA(opcode.mode);
                break;

            case 0xA2:
            case 0xA6:
            case 0xB6:
            case 0xAE:
            case 0xBE:
                op_LDX(opcode.mode);
                break;

            case 0x85:
            case 0x95:
            case 0x8D:
            case 0x9D:
            case 0x99:
            case 0x81:
            case 0x91:
                op_STA(opcode.mode);
                break;

            case 0xAA:
                op_TAX();
                break;

            // BRK
            case 0x00:
                set_status_flag(BREAK_FLAG);
                return;
                break;
        }

        // If the program counter has not been updated by the instruction
        // then we need to update it manually
        if (old_program_counter == program_counter) {
            program_counter += opcode.bytes - 1;
        }
    }
}