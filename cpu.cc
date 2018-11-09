#include "cpu.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "isa.h"

namespace gvm {
void CPU::LoadProgram(uint32_t start, const std::vector<Word>& program) {
  start = start / kWordSize;
  assert(!program.empty());
  assert(program.size() + start < kTotalWords);
  for (uint32_t idx = start, i = 0; i < program.size(); ++idx, i++) {
    mem_[idx] = program[i];
  }
}

void CPU::Run() {
  Run(0);
}

void CPU::Run(uint32_t start) {
  assert(start % kTotalWords == 0);
  start = start / kWordSize;
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
      case ISA::MOV_RI: {
        uint32_t v = word >> 12;
        if (((v >> 11) & 0x01) == 1) v = 0xFFFFF000 | v;
        reg_[(word >> 8) & 0xF] = v;
        break;
      }
      case ISA::LOAD_RR:
        reg_[(word >> 8) & 0xF] = mem_[reg_[(word >> 12) & 0xF]/kWordSize];
        break;
      case ISA::LOAD_RI:
        reg_[(word >> 8) & 0xF] = mem_[((word >> 12) & 0xFFFFF)/kWordSize];
        break;
      case ISA::STOR_RR:
        mem_[reg_[(word >> 8) & 0xF]/kWordSize] = reg_[(word >> 12) & 0xF];
        break;
      case ISA::STOR_RI:
        mem_[((word >> 8) & 0xFFFFF)/kWordSize] = reg_[word >> 28];
        break;
      case ISA::ADD_RR: {
        const uint32_t op1 = (word >> 12) & 0xF;
        const uint32_t op2 = (word >> 16) & 0xF;
        reg_[(word >> 8) & 0xF] = reg_[op1] + reg_[op2];
        break;
      }
      case ISA::SUB_RR: {
        const uint32_t op2 = (~reg_[(word >> 16) & 0xF] + 1);
        reg_[(word >> 8) & 0xF] = reg_[(word >> 12) & 0xF] + op2;
        break;
      }
      case ISA::JMP:
        pc_ = (((word >> 8) & 0xFFFFFF)/kWordSize) - 1;
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
    const auto r = reg_[i];
    r >> 31 == 1 ? ss << int32_t(r)
                 : ss << r;
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

  ss << "Memory from 0x" << std::hex << from << " to 0x" << std::hex << to
     << ":\n";
  for (uint32_t i = from; i <= to; i+=4) {
    ss << "0x" << std::hex << i << ": " << std::dec << mem_[i/kWordSize] << "\n";
  }
  return ss.str();
}

}  // namespace gvm
