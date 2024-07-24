#include "cpu.h"

#include <vector>

CPU::CPU() {
    register_a = 0;
    register_x = 0;
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
    status = 0;

    program_counter = mem_read_u16(0xfffc);
}

void CPU::load_and_run(std::vector<uint8_t> &program) {
    load(program);
    reset();
    run();
}

void CPU::run() {
    while (1) {
        uint8_t opcode = mem_read(program_counter);
        program_counter++;

        switch (opcode) {
            // We need to add braces because we are declaring a variable
            // within the case statement.
            case 0xA9: {
                auto param = mem_read(program_counter);
                lda(param);
            } break;
            case 0xA2: {
                auto param = mem_read(program_counter);
                ldx(param);
            } break;
            case 0xAA:
                tax();
                break;
            case 0xE8:
                inx();
                break;
            case 0x00:
                return;
                break;
        }
    }
}

void CPU::lda(uint8_t value) {
    register_a = value;
    update_zero_and_negative_flags(register_a);
}

void CPU::ldx(uint8_t value) {
    register_x = value;
    update_zero_and_negative_flags(register_x);
}

void CPU::tax() {
    register_x = register_a;
    update_zero_and_negative_flags(register_x);
}

void CPU::inx() {
    register_x++;
    update_zero_and_negative_flags(register_x);
}

void CPU::update_zero_and_negative_flags(uint8_t register_to_check) {
    set_zero_flag(register_to_check);
    set_negative_flag(register_to_check);
}

// This sets the zero flag based on if the result of the
// last exectuted instruction was zero or not.
void CPU::set_zero_flag(uint8_t register_to_check) {
    if (register_to_check == 0) {
        // set the zero flag
        status |= 0x02;
    } else {
        // clear the zero flag
        status &= 0xfd;
    }
}

// This sets the negative flag based on if the 7th bit of the
// last exectuted instruction was set or not.
void CPU::set_negative_flag(uint8_t register_to_check) {
    // 0x80 is the 7th bit set (2^7 = 128 = 0x80)
    // if the 7th bit is set, then the number is negative
    if ((register_to_check & 0x80) != 0) {
        // set the negative flag
        status |= 0x80;
    } else {
        // clear the negative flag
        status &= 0x7f;
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