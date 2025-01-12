#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

#include "cpu/cpu.h"

using json = nlohmann::json;
using namespace std;

template <typename T>
void compare(string title, T check, T expected);

int main() {
    for (int i = 0; i < 0x01; i++) {
        stringstream ss;
        // 7 = 2 for the hex code + len('.json')
        ss << "../ProcessorTests/6502/v1/" << setfill('0') << setw(2)
           << std::hex << i << ".json";

        string file_name = ss.str();

        ifstream file_stream;

        file_stream.open(file_name);

        if (file_stream.fail()) {
            cerr << "Failed to open the file, " << file_name << endl;
            continue;
        }

        json jobject = json::parse(file_stream);
        file_stream.close();

        string name = jobject[0]["name"];
        stringstream split_string(name);
        string temp;
        vector<uint8_t> program;
        while (getline(split_string, temp, ' ')) {
            int hex_code = stoi(temp, nullptr, 16);
            program.push_back(hex_code);
        }

        CPU cpu;
        cpu.load(program);
        // cout << jobject[0]["initial"]["pc"] << '\n';
        cpu.set_program_counter(jobject[0]["initial"]["pc"]);

        cpu.set_register_a(jobject[0]["initial"]["a"]);
        cpu.set_register_x(jobject[0]["initial"]["x"]);
        cpu.set_register_y(jobject[0]["initial"]["y"]);

        cpu.set_stack_pointer(jobject[0]["initial"]["s"]);

        cpu.set_status(jobject[0]["initial"]["p"]);

        for (auto& val : jobject[0]["initial"]["ram"]) {
            cpu.mem_write(val[0], val[1]);
        }

        cpu.run();

        auto final_result = jobject[0]["final"];
        
        compare<int>("Program Counter", cpu.get_program_counter(), final_result["pc"]);
        
        compare<int>("Register A", cpu.get_register_a(), final_result["a"]);
        compare<int>("Register X", cpu.get_register_x(), final_result["x"]);
        compare<int>("Register Y", cpu.get_register_y(), final_result["y"]);

        compare<int>("Stack pointer", cpu.get_status(), final_result["s"]);

        compare<int>("CPU status", cpu.get_status(), final_result["p"]);


        for (auto & val : final_result["ram"]) {
            uint8_t read_val = cpu.mem_read(val[0]);
            stringstream ss;
            ss << "Ram at " << val[0];

            compare<int>(ss.str(), read_val, val[1]);
        }
        
    }
}

template <typename T>
void compare(string title, T check, T expected) {
    if (check != expected) {
        cerr << title << " is not equivelant.\nExpected: " << expected
             << "\nGot: " << check << '\n';
    }
}
