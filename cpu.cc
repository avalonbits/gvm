#include "cpu.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "isa.h"

namespace gvm {

namespace {
constexpr uint32_t reg1(uint32_t word) {
  return (word >> 8) & 0xF;
}
constexpr uint32_t reg2(uint32_t word) {
  return (word >> 12) & 0xF;
}
constexpr uint32_t reg3(uint32_t word) {
  return (word >> 16) & 0xF;
}

constexpr void SetZ(uint8_t& flag, uint8_t zero) {
  flag = flag | (zero & 1);
}

constexpr void SetN(uint8_t& flag, uint8_t negative) {
  flag = flag | ((negative & 1) << 1);
}

constexpr void SetC(uint8_t& flag, uint8_t carry) {
  flag = flag | ((carry & 1) << 2);
}

constexpr bool IsSetZ(uint8_t flag) {
  return static_cast<bool>(flag & 1);
}

constexpr bool IsSetN(uint8_t flag) {
  return static_cast<bool>(flag & 2);
}

constexpr bool IsSetC(uint8_t flag) {
  return static_cast<bool>(flag & 4);
}

}  // namespace

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
    switch (word & 0xFF) {  // first 8 bits define the instruction.
      case ISA::HALT:
        return;
        break;
      case ISA::NOP:
        break;
      case ISA::MOV_RR:
        reg_[reg1(word)] = reg_[reg2(word)];
        break;
      case ISA::MOV_RI: {
        uint32_t v = word >> 12;
        if (((v >> 11) & 1) == 1) v = 0xFFFFF000 | v;
        reg_[reg1(word)] = v;
        break;
      }
      case ISA::LOAD_RR:
        reg_[reg1(word)] = mem_[reg_[reg2(word)]/kWordSize];
        break;
      case ISA::LOAD_RI:
        reg_[reg1(word)] = mem_[((word >> 12) & 0xFFFFF)/kWordSize];
        break;
      case ISA::STOR_RR:
        mem_[reg_[reg1(word)]/kWordSize] = reg_[reg2(word)];
        break;
      case ISA::STOR_RI:
        mem_[((word >> 12) & 0xFFFFF)/kWordSize] = reg_[reg1(word)];
        break;
      case ISA::ADD_RR: {
        const uint32_t v = reg_[reg2(word)] + reg_[reg3(word)];
        reg_[reg1(word)] = v;
        SetZ(sflags_, v == 0);
        SetN(sflags_, v >> 31);
        break;
      }
      case ISA::SUB_RR: {
        const uint32_t op2 = (~reg_[reg3(word)] + 1);
        const uint32_t v = reg_[reg2(word)] + op2;
        reg_[reg1(word)] = v;
        SetZ(sflags_, v == 0);
        SetN(sflags_, v >> 31);
        break;
      }
      case ISA::JMP: {
        uint32_t v = word >> 8;
        if (((word >> 23) & 1) == 1) {
          v = ~v + 1;
          pc_ = pc_ - v/kWordSize - 1;
        } else {
          pc_ = pc_ + v/kWordSize - 1;
        }
        break;
      }
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
