#include "cpu.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
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
  Run(0);
}

void CPU::Run(uint32_t start) {
  assert(start < kTotalWords);
  for (pc_ = start; pc_ < kTotalWords; ++pc_) {
    const auto& word = mem_[pc_];
    switch (word & 0xFF) {  // first 8 bits define the instruction
      case ISA::HALT:
        return;
        break;
      case ISA::NOP:
        break;
      case ISA::MOV_RR:
        reg_[(word >> 8) & 0xF] = reg_[(word >> 12) & 0xF];
        break;
      case ISA::MOV_RI:
        reg_[(word >> 8) & 0xF] = word >> 12;
        break;
      case ISA::LOAD_RR:
        break;
      case ISA::LOAD_RI:
        reg_[(word >> 8) & 0xF] = mem_[(word >> 12)];
        break;
      case ISA::STOR_RR:
        break;
      case ISA::STOR_RI:
        mem_[(word >> 8) & 0xFFFFF] = reg_[word >> 28];
        break;
      case ISA::ADD_RR:
        reg_[(word >> 8) & 0xF] = reg_[(word >> 12) & 0xF] + reg_[(word >> 16) & 0xF];
        break;
      default:
        assert(false);
   }
 }
}

const std::string CPU::PrintRegisters() {
  std::stringstream ss;

  ss << "[";
  for (uint32_t i = 0; i < kRegCount; ++i) {
    if (i != 0) ss << " ";
    ss << reg_[i];
  }
  ss << "]\n";
  return ss.str();
}

const std::string CPU::PrintMemory(uint32_t from, uint32_t to) {
  assert(from <= to);
  assert(from % 4 == 0);
  assert(to % 4 == 0);
  assert(from < kTotalWords);
  assert(to < kTotalWords);

  std::stringstream ss;

  ss << "Memofry from 0x" << std::hex << from << " to 0x" << std::hex << to
     << ":\n";
  for (uint32_t i = from; i <= to; i += 4) {
    ss << "0x" << std::hex << i << ": " << std::dec << mem_[i] << "\n";
  }
  return ss.str();
}

}  // namespace gvm
