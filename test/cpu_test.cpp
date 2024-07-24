#include <gtest/gtest.h>
#include "cpu.h"
#include <vector>

// Demonstrate some basic assertions.
TEST(CPUTest, LDA) {
  CPU cpu;
  // load the number 5 into the accumulator
  std::vector<uint8_t> program = {0xa9, 0x05, 0x00};
  cpu.load_and_run(program);
  ASSERT_EQ(cpu.register_a, 0x05);

  // making sure the zero flag is not set
  ASSERT_EQ(cpu.status & 0x02, 0x00);
  // making sure the negative flag is not set
  ASSERT_EQ(cpu.status & 0x80, 0x00);
}

TEST(CPUTest, TAX) {
  CPU cpu;
  // load the number 5 into the accumulator and transfer it to the x register
  std::vector<uint8_t> program = {0xa9, 0x05, 0xaa, 0x00};
  cpu.load_and_run(program);
  ASSERT_EQ(cpu.register_x, 0x05);

  // making sure the zero flag is not set
  ASSERT_EQ(cpu.status & 0x02, 0x00);
  // making sure the negative flag is not set
  ASSERT_EQ(cpu.status & 0x80, 0x00);
}

// Just a test that I wasn't sure about
TEST(CPUTest, OverflowTest) {
  CPU cpu;
  
  // increments twice and then C++ wraps it to 0 when it overflows
  std::vector<uint8_t> program = {0xA2, 0xFF, 0xE8, 0xE8, 0x00};
  cpu.load_and_run(program);
  ASSERT_EQ(cpu.register_x, 1);
}

TEST(CPUTest, FiveOpsWorkingTogether) {
  CPU cpu;
  // loads c0 into a register
  // transfers it to x register
  // increments
  std::vector<uint8_t> program = {0xa9, 0xc0, 0xaa, 0xe8, 0x00};
  cpu.load_and_run(program);

  ASSERT_EQ(cpu.register_x, 0xc1);
}