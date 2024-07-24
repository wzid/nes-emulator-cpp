#pragma once
#include <cstdint>
#include <vector>

enum AddressingMode {
    IMMEDIATE,
    ZEROPAGE,
    ZEROPAGE_X,
    ZEROPAGE_Y,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    INDIRECT_X,
    INDIRECT_Y,
    NONE_ADDRESSING,
};

const static char *addressing_mode_name[10] = {
    "Immediate",  "Zero Page",      "Zero Page X", "Zero Page Y",
    "Absolute",   "Absolute X",     "Absolute Y",  "Indirect X",
    "Indirect Y", "None Addressing"};

class CPU {
   public:
    CPU();

    void reset();
    void load_and_run(std::vector<uint8_t> &program);
    void load(std::vector<uint8_t> &program);
    void run();

    uint8_t mem_read(uint16_t address);
    void mem_write(uint16_t address, uint8_t data);

    uint16_t mem_read_u16(uint16_t pos);
    void mem_write_u16(uint16_t pos, uint16_t data);

    uint8_t get_register_a() { return register_a; }
    uint8_t get_register_x() { return register_x; }
    uint8_t get_register_y() { return register_y; }
    uint8_t get_status() { return status; }

   private:
    uint16_t get_operand_address(AddressingMode &mode);

    void inx();
    void lda(AddressingMode &mode);
    void ldx(AddressingMode &mode);
    void sta(AddressingMode &mode);
    void tax();
    void update_zero_and_negative_flags(uint8_t register_to_check);

    // We have these so I can comment them more efficently
    void set_zero_flag(uint8_t register_to_check);
    void set_negative_flag(uint8_t register_to_check);

    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    // 64 KiB
    uint8_t memory[0xffff];
};