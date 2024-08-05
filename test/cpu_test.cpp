#include "cpu.h"

#include <gtest/gtest.h>

#include <vector>

// Demonstrate some basic assertions.
TEST(CPUTest, LDA) {
    CPU cpu;
    // load the number 5 into the accumulator
    std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
    cpu.load_and_run(program);
    ASSERT_EQ(cpu.get_register_a(), 0x05);

    // making sure the zero flag is not set
    ASSERT_TRUE(!cpu.is_status_flag_set(ZERO_FLAG));
    // making sure the negative flag is not set
    ASSERT_TRUE(!cpu.is_status_flag_set(NEGATIVE_FLAG));
}

TEST(CPUTest, TAX) {
    CPU cpu;
    // load the number 5 into the accumulator and transfer it to the x register
    std::vector<uint8_t> program = {0xA9, 0x05, 0xAA, 0x00};
    cpu.load_and_run(program);
    ASSERT_EQ(cpu.get_register_x(), 0x05);

    // making sure the zero flag is not set
    ASSERT_TRUE(!cpu.is_status_flag_set(ZERO_FLAG));
    // making sure the negative flag is not set
    ASSERT_TRUE(!cpu.is_status_flag_set(NEGATIVE_FLAG));
}

// Just a test that I wasn't sure about
TEST(CPUTest, OverflowTest) {
    CPU cpu;

    // increments twice and then C++ wraps it to 0 when it overflows
    std::vector<uint8_t> program = {0xA2, 0xFF, 0xE8, 0xE8, 0x00};
    cpu.load_and_run(program);
    ASSERT_EQ(cpu.get_register_x(), 1);
}

TEST(CPUTest, FiveOpsWorkingTogether) {
    CPU cpu;
    // loads c0 into a register
    // transfers it to x register
    // increments
    std::vector<uint8_t> program = {0xA9, 0xC0, 0xAA, 0xE8, 0x00};
    cpu.load_and_run(program);

    ASSERT_EQ(cpu.get_register_x(), 0xc1);
}

TEST(CPUTest, TestLDAFromMemory) {
    CPU cpu;
    cpu.mem_write(0x10, 0x55);

    std::vector<uint8_t> program = {0xA5, 0x10, 0x00};
    cpu.load_and_run(program);

    ASSERT_EQ(cpu.get_register_a(), 0x55);
}

TEST(CPUTest, TEST_ADC) {
    CPU cpu;

    std::vector<uint8_t> program = {0XA9, 0x05, 0x69, 0x05, 0x00};
    cpu.load_and_run(program);

    // we added 0x05 + 0x05 which is 0x0A
    ASSERT_EQ(cpu.get_register_a(), 0x0A);
}

TEST(CPUTest, TEST_ADC_withCarry) {
    CPU cpu;

    std::vector<uint8_t> program = {0XA9, 0x05, 0x38, 0x69, 0x05, 0x00};
    cpu.load_and_run(program);

    // we added 0x05 + 0x05 + 1 which is 0x0A
    ASSERT_EQ(cpu.get_register_a(), 0x0A + 1);
}

TEST(CPUTest, TEST_AND) {
    CPU cpu;

    std::vector<uint8_t> program = {0XA9, 0x05, 0x29, 0x06, 0x00};
    cpu.load_and_run(program);

    // we did 0x05 & 0x06 which is 0x04
    ASSERT_EQ(cpu.get_register_a(), 0x04);
}

TEST(CPUTest, TEST_ASL) {
    CPU cpu;

    std::vector<uint8_t> program = {0XA9, 0x05, 0x0A, 0x00};
    cpu.load_and_run(program);

    ASSERT_EQ(cpu.get_register_a(), 0x05 << 1);
}

TEST(CPUTest, TEST_CMP) {
    CPU cpu;

    std::vector<uint8_t> program = {0XA9, 0x05, 0xC9, 0x05, 0x00};
    cpu.load_and_run(program);

    // we compared 0x05 with 0x05, which should set the zero flag since they are
    // equal
    ASSERT_TRUE(cpu.is_status_flag_set(ZERO_FLAG));
}

TEST(CPUTest, TEST_JMP) {
    CPU cpu;

    std::vector<uint8_t> program = {0x4C, 0x05, 0x80, 0xEA,
                                    0x00, 0xA9, 0x09, 0x00};
    cpu.load_and_run(program);

    // trying to jump to an instruction where we load 0x09 into register_a
    ASSERT_EQ(cpu.get_register_a(), 0x09);
}

TEST(CPUTest, TEST_PHP_AND_PLP) {
    CPU cpu;

    std::vector<uint8_t> program = {0x08, 0xA9, 0x00, 0x28, 0x00};
    cpu.load_and_run(program);

    // pushing the defualt status, then setting the zero flag via LDA
    // then pulling the status from the stack, which the zero flag should not be
    // set in
    ASSERT_TRUE(!cpu.is_status_flag_set(ZERO_FLAG));
}

TEST(CPUTest, TEST_PHA_AND_PLA) {
    CPU cpu;

    std::vector<uint8_t> program = {0xA9, 0xFC, 0x48, 0xA9, 0x06, 0x68, 0x00};
    cpu.load_and_run(program);

    // We are loading 0xFC into register_a, then pushing to the stack
    // we then load 0x06 into register_a and pull the accumulator from the stack
    // which should set register_a back to 0xFC
    ASSERT_EQ(cpu.get_register_a(), 0xFC);
}

TEST(CPUTest, TEST_ROL_Accumulator) {
    CPU cpu;

    std::vector<uint8_t> program = {0xA9, 0xFC, 0x2A, 0x2A, 0x00};
    cpu.load_and_run(program);

    // We are loading 0xFC into register_a, then doing a ROL twice.
    // Once to set the carry flag and second to see if the 0 bit is set to 1
    // because of the carry flag.
    // the result of the rotate should be F1
    ASSERT_EQ(cpu.get_register_a(), 0xF1);
    // The carry flag should also still be set cause there was another 1 at the front
    ASSERT_TRUE(cpu.is_status_flag_set(CARRY_FLAG));
}

TEST(CPUTest, TEST_ROL) {
    CPU cpu;

    cpu.mem_write(0x8030, 0x8F);
    std::vector<uint8_t> program = {0x2E, 0x30, 0x80, 0x2E, 0x30, 0x80, 0x00};
    cpu.load_and_run(program);

    auto value = cpu.mem_read(0x8030);
    // We are rotating the value at 0x8030 twice.
    // it should result in 0x3D
    ASSERT_EQ(value, 0x3D);
    // carry flag should NOT be set in this situation
    // there is not another leading 1
    ASSERT_TRUE(!cpu.is_status_flag_set(CARRY_FLAG));
}

TEST(CPUTest, TEST_ROR_Accumulator) {
    CPU cpu;

    std::vector<uint8_t> program = {0xA9, 0x4F, 0x6A, 0x6A, 0x00};
    cpu.load_and_run(program);

    // We are loading 0x4F into register_a, then doing a ROR twice.
    // Once to set the carry flag and second to see if the 7 bit is set to 1
    // because of the carry flag.
    // the result of the rotate should be 0x93
    ASSERT_EQ(cpu.get_register_a(), 0x93);
    // The carry flag should also still be set cause there was another 1 at the end
    ASSERT_TRUE(cpu.is_status_flag_set(CARRY_FLAG));
}

TEST(CPUTest, TEST_ROR) {
    CPU cpu;

    cpu.mem_write(0x8030, 0xF9);
    std::vector<uint8_t> program = {0x6E, 0x30, 0x80, 0x6E, 0x30, 0x80, 0x00};
    cpu.load_and_run(program);

    auto value = cpu.mem_read(0x8030);
    // We are rotating the value at 0x8030 twice.
    // it should result in 0xBE
    ASSERT_EQ(value, 0xBE);
    ASSERT_TRUE(!cpu.is_status_flag_set(CARRY_FLAG));
}

TEST(CPUTest, TEST_SBC) {
    CPU cpu;

    std::vector<uint8_t> program = {0xA9, 0x55, 0xE9, 0x05, 0x00};
    cpu.load_and_run(program);

    ASSERT_EQ(cpu.get_register_a(), 0x55 - 0x05 - 1);
}

TEST(CPUTest, TEST_SBC_withCarry) {
    CPU cpu;

    std::vector<uint8_t> program = {0xA9, 0x55, 0x38, 0xE9, 0x05, 0x00};
    cpu.load_and_run(program);

    ASSERT_EQ(cpu.get_register_a(), 0x55 - 0x05);
}