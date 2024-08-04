#pragma once
#include <cstdint>
#include <vector>

#include "opcode.h"

const static uint8_t CARRY_FLAG = 1 << 0;              // 00000001
const static uint8_t ZERO_FLAG = 1 << 1;               // 00000010
const static uint8_t INTERRUPT_DISABLE_FLAG = 1 << 2;  // 00000100
const static uint8_t DECIMAL_MODE_FLAG = 1 << 3;       // 00001000
const static uint8_t BREAK_FLAG = 1 << 4;              // 00010000
const static uint8_t BREAK2_FLAG = 1 << 5;             // 00100000
const static uint8_t OVERFLOW_FLAG = 1 << 6;           // 01000000
const static uint8_t NEGATIVE_FLAG = 1 << 7;           // 10000000

class CPU {
   public:
    CPU();

    void reset();
    void load_and_run(std::vector<uint8_t> &program);
    void load(std::vector<uint8_t> &program);
    void run();

    bool has_status_flag(uint8_t status_flag);
    void set_status_flag(uint8_t status_flag);
    void clear_status_flag(uint8_t status_flag);
    void set_status_flag_bit(uint8_t status_flag, bool check);

    uint8_t mem_read(uint16_t address);
    void mem_write(uint16_t address, uint8_t data);

    uint16_t mem_read_u16(uint16_t pos);
    void mem_write_u16(uint16_t pos, uint16_t data);

    uint8_t get_register_a() { return register_a; }
    uint8_t get_register_x() { return register_x; }
    uint8_t get_register_y() { return register_y; }
    uint8_t get_status() { return status; }

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

   private:
    uint16_t get_operand_address(AddressingMode &mode);
    void add_to_register_a(uint8_t value);

    [[gnu::always_inline]]
    inline void op_ADC(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_AND(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_ASL_register_a();

    [[gnu::always_inline]]
    inline void op_ASL(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void branch();

    [[gnu::always_inline]]
    inline void op_BIT(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void compare(AddressingMode &mode, uint8_t register_to_compare);

    [[gnu::always_inline]]
    inline void op_DEC(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_DEX();

    [[gnu::always_inline]]
    inline void op_DEY();

    [[gnu::always_inline]]
    inline void op_EOR(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_INC(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_INX();

    [[gnu::always_inline]]
    inline void op_LDA(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_LDX(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_STA(AddressingMode &mode);

    [[gnu::always_inline]]
    inline void op_TAX();

    void update_zero_and_negative_flags(uint8_t register_to_check);

    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    // 64 KiB
    uint8_t memory[0xffff];
};