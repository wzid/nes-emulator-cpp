#include <gtest/gtest.h>
#include "cpu.h"
#include <vector>

// Demonstrate some basic assertions.
TEST(CPUTest, LDA) {
  CPU cpu;
  // load the number 5 into the accumulator
  std::vector<uint8_t> program = {0xa9, 0x05, 0x00};
  cpu.interpret(program);
  ASSERT_EQ(cpu.register_a, 0x05);
  // making sure the zero flag is set
  ASSERT_EQ(cpu.status & 0x02, 0x00);
  // making sure the negative flag is not set
  ASSERT_EQ(cpu.status & 0x80, 0x00);
}