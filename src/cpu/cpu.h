#pragma once
#include <cstdint>
#include <vector>

#include "opcode.h"

const static uint8_t CARRY_FLAG = 1 << 0;              // 00000001
const static uint8_t ZERO_FLAG = 1 << 1;               // 00000010
const static uint8_t INTERRUPT_DISABLE_FLAG = 1 << 2;  // 00000100
const static uint8_t DECIMAL_MODE_FLAG = 1 << 3;       // 00001000
const static uint8_t BREAK_FLAG = 1 << 4;              // 00010000
const static uint8_t ALWAYS_ONE_FLAG = 1 << 5;         // 00100000
const static uint8_t OVERFLOW_FLAG = 1 << 6;           // 01000000
const static uint8_t NEGATIVE_FLAG = 1 << 7;           // 10000000

const static uint16_t STACK = 0x0100;
const static uint8_t STACK_RESET = 0xFD;

class CPU {
   public:
    CPU();

    void reset();
    void load_and_run(std::vector<uint8_t> &program);
    void load(std::vector<uint8_t> &program);
    void run();
    void run_with_callback(void (*callback_function)(CPU &));

    bool is_status_flag_set(uint8_t status_flag);
    void set_status_flag(uint8_t status_flag);
    void clear_status_flag(uint8_t status_flag);
    void set_status_flag_bit(uint8_t status_flag, bool check);

    uint8_t mem_read(uint16_t address);
    void mem_write(uint16_t address, uint8_t data);

    uint16_t mem_read_u16(uint16_t pos);
    void mem_write_u16(uint16_t pos, uint16_t data);

    void stack_push(uint8_t data);
    void stack_push_u16(uint16_t data);

    uint8_t stack_pop();
    uint16_t stack_pop_u16();

    uint8_t get_register_a() { return register_a; }
    uint8_t get_register_x() { return register_x; }
    uint8_t get_register_y() { return register_y; }
    uint8_t get_status() { return status; }
    uint16_t get_program_counter() { return program_counter; }

    void set_register_a(uint8_t value) {
        register_a = value;
        update_zero_and_negative_flags(register_a);
    }

    void set_register_x(uint8_t value) {
        register_x = value;
        update_zero_and_negative_flags(register_x);
    }

    void set_register_y(uint8_t value) {
        register_y = value;
        update_zero_and_negative_flags(register_y);
    }

    void set_program_counter(uint16_t value) {
        program_counter = value;
    }

    void set_stack_pointer(uint8_t value) {
        stack_pointer = value;
    }

    void set_status(uint8_t value) {
        status = value;
    }

   private:
    uint16_t get_operand_address(AddressingMode &mode);
    void add_to_register_a(uint8_t value);

    [[gnu::always_inline]]
    inline void branch();

    [[gnu::always_inline]]
    inline void compare(uint16_t operand_address, uint8_t register_to_compare);

    void update_zero_and_negative_flags(uint8_t register_to_check);

    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    // https://www.nesdev.org/obelisk-6502-guide/registers.html
    // look for Stack Pointer
    uint8_t stack_pointer;
    // 64 KiB
    uint8_t memory[0xffff];
};