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
    for (; pc_ < kTotalWords; ++pc_) {
        const auto& word = mem_[pc_];
        const auto opcode = word & 0xFF;
        const auto dest = (word >> 8) & 0xF;
        const auto op1 = (word >> 12) & 0xF;
        switch (opcode) {  // first 8 bits define the instruction
            case ISA::HALT:
                return;
                break;
            case ISA::NOP:
                break;
            case ISA::LOAD_RR:
                reg_[dest] = reg_[op1];
                break;
            case ISA::LOAD_RI:
                reg_[dest] = op1;
                break;
            case ISA::ADD_RR: {
                const auto op2 = (word >> 12) & 0xF;
                reg_[dest] = reg_[op1] + reg_[op2];
                break;
            }
            default:
                assert(false);
        }
    }
}

}  // namespace gvm
