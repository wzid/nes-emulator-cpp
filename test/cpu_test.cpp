#include <gtest/gtest.h>
#include "cpu.h"
#include <vector>

// Demonstrate some basic assertions.
TEST(CPUTest, LDA) {
  CPU cpu;
  // load the number 5 into the accumulator
  std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
  cpu.load_and_run(program);
  ASSERT_EQ(cpu.get_register_a(), 0x05);

  // making sure the zero flag is not set
  ASSERT_EQ(cpu.get_status() & 0x02, 0x00);
  // making sure the negative flag is not set
  ASSERT_EQ(cpu.get_status() & 0x80, 0x00);
}

TEST(CPUTest, TAX) {
  CPU cpu;
  // load the number 5 into the accumulator and transfer it to the x register
  std::vector<uint8_t> program = {0xA9, 0x05, 0xAA, 0x00};
  cpu.load_and_run(program);
  ASSERT_EQ(cpu.get_register_x(), 0x05);

  // making sure the zero flag is not set
  ASSERT_EQ(cpu.get_status() & 0x02, 0x00);
  // making sure the negative flag is not set
  ASSERT_EQ(cpu.get_status() & 0x80, 0x00);
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

// TODO: Once SEC opcode is finished, actually test with carry
TEST(CPUTest, TEST_ADC) {
  CPU cpu;
  
  std::vector<uint8_t> program = {0XA9, 0x05, 0x69, 0x05, 0x00};
  cpu.load_and_run(program);

  // we added 0x05 + 0x05 which is 0x0A
  ASSERT_EQ(cpu.get_register_a(), 0x0A);
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

  // we did 0x05 & 0x06 which is 0x04
  ASSERT_EQ(cpu.get_register_a(), 0x05 >> 1);
}