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

void CPU::branch() {
    uint8_t jump = mem_read(program_counter);
    // since the location is relative to the branch we have to increment by
    // the program counter and then 1 to start at the next instruction
    uint8_t jump_addr = program_counter + jump + 1;

    program_counter = jump_addr;
}

void CPU::compare(AddressingMode &mode, uint8_t register_to_compare) {
    auto addr = get_operand_address(mode);
    auto value = mem_read(addr);

    set_status_flag_bit(CARRY_FLAG, register_to_compare >= value);

    // Although we have to set ZERO based on if register_to_compare == value
    // We can just reduce register_to_compare by value, and if its zero
    // just set the zero flag, which is what this function does already
    update_zero_and_negative_flags(register_to_compare - value);
}

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
        // does not need an address if it is implied or accumulator
        case ACCUMULATOR:
        case IMPLIED:
            return 0;
    }
}

void CPU::run() {
    while (1) {
        uint8_t hex_code = mem_read(program_counter);
        program_counter++;
        uint16_t old_program_counter = program_counter;

        OpCode opcode = opcodes[hex_code];
        uint16_t addr = get_operand_address(opcode.mode);

        switch (opcode.mnemonic) {
            case ADC: {
                auto value = mem_read(addr);
                add_to_register_a(value);
            } break;
            case AND: {
                auto value = mem_read(addr);
                set_register_a(register_a & value);
            } break;
            case ASL_accumulator:
                set_status_flag_bit(CARRY_FLAG, register_a >> 7);
                set_register_a(register_a >> 1);
                break;
            case ASL: {
                auto value = mem_read(addr);

                set_status_flag_bit(CARRY_FLAG, value >> 7);

                value >>= 1;
                mem_write(addr, value);
                update_zero_and_negative_flags(value);
            } break;
            case BCC:
                if (!has_status_flag(CARRY_FLAG)) {
                    branch();
                }
                break;
            case BCS:
                if (has_status_flag(CARRY_FLAG)) {
                    branch();
                }
                break;
            case BEQ:
                if (has_status_flag(ZERO_FLAG)) {
                    branch();
                }
                break;
            case BIT: {
                auto value = mem_read(addr);

                if ((register_a & value) == 0) {
                    set_status_flag(ZERO_FLAG);
                } else {
                    clear_status_flag(ZERO_FLAG);
                }

                //  Bits 7 and 6 of the value from memory are copied into the N
                //  and V flags.

                set_status_flag_bit(NEGATIVE_FLAG, value & (1 << 7));
                set_status_flag_bit(OVERFLOW_FLAG, value & (1 << 6));
            } break;
            case BMI:
                if (has_status_flag(NEGATIVE_FLAG)) {
                    branch();
                }
                break;
            case BNE:
                if (!has_status_flag(ZERO_FLAG)) {
                    branch();
                }
                break;
            case BPL:
                if (!has_status_flag(NEGATIVE_FLAG)) {
                    branch();
                }
                break;
            case BVC:
                if (!has_status_flag(OVERFLOW_FLAG)) {
                    branch();
                }
                break;

            case BVS:
                if (has_status_flag(OVERFLOW_FLAG)) {
                    branch();
                }
                break;
            case CLC:
                clear_status_flag(CARRY_FLAG);
                break;

            case CLD:
                clear_status_flag(DECIMAL_MODE_FLAG);
                break;

            case CLI:
                clear_status_flag(INTERRUPT_DISABLE_FLAG);
                break;
            case CLV:
                clear_status_flag(OVERFLOW_FLAG);
                break;
            case CMP:
                compare(opcode.mode, register_a);
                break;
            case CPX:
                compare(opcode.mode, register_x);
                break;
            case CPY:
                compare(opcode.mode, register_y);
                break;
            case DEC: {
                auto value = mem_read(addr);

                value--;
                mem_write(addr, value);

                update_zero_and_negative_flags(value);
            } break;
            case DEX:
                register_x--;
                update_zero_and_negative_flags(register_x);
                break;
            case DEY:
                register_y--;
                update_zero_and_negative_flags(register_y);
                break;
            case EOR: {
                auto value = mem_read(addr);

                register_a ^= value;

                update_zero_and_negative_flags(register_a);
            } break;
            case INC: {
                auto value = mem_read(addr);
                value++;

                mem_write(addr, value);
                update_zero_and_negative_flags(value);
            } break;
            case INX:
                register_x++;
                update_zero_and_negative_flags(register_x);
                break;
            case LDA: {
                auto value = mem_read(addr);
                set_register_a(value);
            } break;
            case LDX: {
                auto value = mem_read(addr);
                set_register_x(value);
            } break;
            case STA:
                mem_write(addr, register_a);
                break;
            case TAX:
                set_register_x(register_a);
                break;
            case BRK:
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