#include "cpu.h"
#include <vector>

CPU::CPU() {
    register_a = 0;
    status = 0;
    program_counter = 0;
}

void CPU::interpret(std::vector<uint8_t> &program) {
    while (1) {
        auto opcode = program[program_counter];
        program_counter++;

        switch (opcode) {
            // LDAd
            case 0xa9: {
                    auto param = program[program_counter];
                    program_counter++;
                    register_a = param;

                    set_zero_flag(register_a);
                    set_negative_flag(register_a);
                }
                break;
            case 0x00:
                return;
                break;
        }
    }
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