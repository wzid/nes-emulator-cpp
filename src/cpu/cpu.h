#include <cstdint>
#include <vector>

class CPU {
    public:
        uint8_t register_a;
        uint8_t register_x;
        uint8_t status;
        uint16_t program_counter;
        // 64 KiB
        uint8_t memory[0xffff];

        CPU();
        
        void reset();
        void load_and_run(std::vector<uint8_t> &program);
        void load(std::vector<uint8_t> &program);
        void run();
    private:
        void lda(uint8_t value);
        void ldx(uint8_t value);
        void tax();
        void inx();
        void update_zero_and_negative_flags(uint8_t register_to_check);
        
        // We have these so I can comment them more efficently
        void set_zero_flag(uint8_t register_to_check);
        void set_negative_flag(uint8_t register_to_check);

        uint8_t mem_read(uint16_t address);
        void mem_write(uint16_t address, uint8_t data);

        uint16_t mem_read_u16(uint16_t pos);
        void mem_write_u16(uint16_t pos, uint16_t data);
};