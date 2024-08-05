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
    register_y = 0;
    stack_pointer = STACK_RESET;

    // why is 3rd bit to right set?
    status = 0b00100100;

    program_counter = mem_read_u16(0xfffc);
}

void CPU::load_and_run(std::vector<uint8_t> &program) {
    load(program);
    reset();
    run();
}

bool CPU::is_status_flag_set(uint8_t status_flag) {
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

    if (is_status_flag_set(CARRY_FLAG)) {
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

#include <iostream>
void CPU::stack_push(uint8_t data) {
    uint16_t addr = STACK + stack_pointer;
    mem_write(STACK + stack_pointer, data);
    stack_pointer--;
}

// have to manually do it instead of mem_write_u16 because the stack works
// backwards and we want lower to be the first one to be "popped"
void CPU::stack_push_u16(uint16_t data) {
    uint8_t higher = data >> 8;
    uint8_t lower = (data & 0xff);
    stack_push(higher);
    stack_push(lower);
}

uint8_t CPU::stack_pop() {
    stack_pointer++;
    return mem_read(STACK + stack_pointer);
}

uint16_t CPU::stack_pop_u16() {
    uint16_t lower = stack_pop();
    uint16_t higher = stack_pop();

    return (higher << 8) | lower;
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
        case INDIRECT: {
            // JMP is the only instruction with INDIRECT addressing mode
            // https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
            uint16_t addr = mem_read_u16(program_counter);

            uint16_t result = 0;
            // bug in 6502 processors - see link above for more information
            if ((addr & 0x00FF) == 0x00FF) {
                uint16_t lower = mem_read(addr);
                uint16_t higher = mem_read(addr & 0xFF00);
                result = (higher << 8) | lower;
            } else {
                result = mem_read_u16(addr);
            }

            return result;
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
                set_register_a(register_a << 1);
                break;
            case ASL: {
                auto value = mem_read(addr);

                set_status_flag_bit(CARRY_FLAG, value >> 7);

                value <<= 1;
                mem_write(addr, value);
                update_zero_and_negative_flags(value);
            } break;
            case BCC:
                if (!is_status_flag_set(CARRY_FLAG)) {
                    branch();
                }
                break;
            case BCS:
                if (is_status_flag_set(CARRY_FLAG)) {
                    branch();
                }
                break;
            case BEQ:
                if (is_status_flag_set(ZERO_FLAG)) {
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
                if (is_status_flag_set(NEGATIVE_FLAG)) {
                    branch();
                }
                break;
            case BNE:
                if (!is_status_flag_set(ZERO_FLAG)) {
                    branch();
                }
                break;
            case BPL:
                if (!is_status_flag_set(NEGATIVE_FLAG)) {
                    branch();
                }
                break;
            case BVC:
                if (!is_status_flag_set(OVERFLOW_FLAG)) {
                    branch();
                }
                break;

            case BVS:
                if (is_status_flag_set(OVERFLOW_FLAG)) {
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
            case JMP:
                program_counter = addr;
                break;
            case JSR:
                // The JSR instruction pushes the address (minus one) of the
                // return point on to the stack and then sets the program
                // counter to the target memory address.
                stack_push_u16(program_counter + 2 - 1);
                program_counter = addr;
                break;
            case LDA: {
                auto value = mem_read(addr);
                set_register_a(value);
            } break;
            case LDX: {
                auto value = mem_read(addr);
                set_register_x(value);
            } break;
            case LDY: {
                auto value = mem_read(addr);
                set_register_y(addr);
            } break;
            case LSR_accumulator:
                set_status_flag_bit(CARRY_FLAG, register_a & 1);
                set_register_a(register_a >> 1);
                break;
            case LSR: {
                auto value = mem_read(addr);
                set_status_flag_bit(CARRY_FLAG, value & 1);
                value >>= 1;
                mem_write(addr, value);
                update_zero_and_negative_flags(value);
            } break;
            case NOP:
                // do nothing :)
                break;
            case ORA: {
                auto value = mem_read(addr);
                set_register_a(register_a | value);
            } break;
            case PHA:
                stack_push(register_a);
                break;
            case PHP: {
                uint8_t status_clone = status;
                status_clone |= BREAK_FLAG;
                status_clone |= ALWAYS_ONE_FLAG;
                stack_push(status_clone);
            } break;
            case PLA: {
                uint8_t accumulator = stack_pop();
                set_register_a(accumulator);
            } break;
            case PLP: {
                uint8_t processor_status = stack_pop();
                status = processor_status;
                clear_status_flag(BREAK_FLAG);
                set_status_flag(ALWAYS_ONE_FLAG);
            } break;
            case ROL_accumulator: {
                uint8_t new_register_a = register_a;
                uint8_t carry_was_set = is_status_flag_set(CARRY_FLAG);

                set_status_flag_bit(CARRY_FLAG, new_register_a >> 7);

                new_register_a <<= 1;
                if (carry_was_set) {
                    new_register_a |= 1;
                }
                set_register_a(new_register_a);
            } break;
            case ROL: {
                auto data = mem_read(addr);
                uint8_t carry_was_set = is_status_flag_set(CARRY_FLAG);
                set_status_flag_bit(CARRY_FLAG, data >> 7);

                data <<= 1;
                if (carry_was_set) {
                    data |= 1;
                }
                mem_write(addr, data);
                update_zero_and_negative_flags(data);
            } break;
            case ROR_accumulator: {
                uint8_t new_register_a = register_a;
                uint8_t carry_was_set = is_status_flag_set(CARRY_FLAG);

                set_status_flag_bit(CARRY_FLAG, new_register_a & 1);

                new_register_a >>= 1;
                if (carry_was_set) {
                    // 0x80 (set 7th bit to carry)
                    new_register_a |= 0b10000000;
                }
                set_register_a(new_register_a);
            } break;
            case ROR: {
                auto data = mem_read(addr);
                uint8_t carry_was_set = is_status_flag_set(CARRY_FLAG);
                set_status_flag_bit(CARRY_FLAG, data & 1);

                data >>= 1;
                if (carry_was_set) {
                    // 0x80 (set 7th bit to carry)
                    data |= 0b10000000;
                }
                mem_write(addr, data);
                update_zero_and_negative_flags(data);
            } break;
            case RTI:
                status = stack_pop();
                clear_status_flag(BREAK_FLAG);
                set_status_flag(ALWAYS_ONE_FLAG);

                program_counter = stack_pop_u16();
                break;
            case RTS:
                program_counter = stack_pop_u16() + 1;
                break;
            case SBC: {
                auto value = mem_read(addr);
                add_to_register_a(-value - 1);
            } break;
            case SEC:
                set_status_flag(CARRY_FLAG);
                break;
            case SED:
                set_status_flag(DECIMAL_MODE_FLAG);
                break;
            case SEI:
                set_status_flag(INTERRUPT_DISABLE_FLAG);
                break;
            case STA:
                mem_write(addr, register_a);
                break;
            case STX:
                mem_write(addr, register_x);
                break;
            case STY:
                mem_write(addr, register_y);
                break;
            case TAX:
                set_register_x(register_a);
                break;
            case TAY:
                set_register_y(register_a);
                break;
            case TSX:
                set_register_x(stack_pointer);
                break;
            case TXA:
                set_register_a(register_x);
                break;
            case TXS:
                stack_pointer = register_x;
                break;
            case TYA:
                set_register_a(register_y);
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