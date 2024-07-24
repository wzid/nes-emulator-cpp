#include <cstdint>
#include <vector>

class CPU {
    public:
        uint8_t register_a;
        uint8_t status;
        uint8_t program_counter;

        CPU();

        void interpret(std::vector<uint8_t> &program);
    private:
        void set_zero_flag(uint8_t register_to_check);
        void set_negative_flag(uint8_t register_to_check);
};