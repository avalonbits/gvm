#include "cpu.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

#include "isa.h"

namespace gvm {
void CPU::LoadProgram(uint32_t start, const std::vector<Word>& program) {
    assert(!program.empty());
    assert(program.size() + start < kTotalWords);
    for (uint32_t i = start; i < program.size(); ++i) {
        mem_[i] = program[i];
    }
}

void CPU::Run() {
    uint32_t count = 1000000000;
    uint32_t count_save = count;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto pc = pc_; count > 0 && pc_ < kTotalWords; ++pc_) {
        const auto& word = mem_[pc_];
        switch (word & 0xFF) {  // first 8 bits define the instruction
            case ISA::HALT:
                pc_ = pc;
                --count;
                break;
            case ISA::NOP:
                break;
            case ISA::LOAD_RR:
                reg_[(word >> 8) & 0xF] = reg_[(word >> 12) & 0xF];
                break;
            case ISA::LOAD_RI:
                reg_[(word >> 8) & 0xF] = word >> 12;
                break;
            case ISA::ADD_RR:
                reg_[(word >> 8) & 0xF] = reg_[(word >> 12) & 0xF] + reg_[(word >> 16) & 0xF];
                break;
            default:
                assert(false);
        }
    }
    std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cerr << static_cast<double>(elapsed.count()) / 6 / count_save;
}

}  // namespace gvm
