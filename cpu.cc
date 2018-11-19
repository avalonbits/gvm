#include "cpu.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "isa.h"

namespace gvm {

static const uint32_t kVideoMemReg = 0x00;
static const uint32_t kVideoMemStart = 0x400;
static const uint32_t kVideoMemSizeWords = 800 * 450;
static const uint32_t kVideoMemEnd = kVideoMemStart + kVideoMemSizeWords;
static const int kFrameBufferW = 800;
static const int kFrameBufferH = 450;

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
constexpr uint32_t v16bit(uint32_t word) {
  return (word >> 16) & 0xFFFF;
}
constexpr uint32_t ext16bit(uint32_t word) {
  return (0x00008000 & word) ? (0xFFFF0000 | word) : word;
}

constexpr void SetZ(uint8_t& flag, bool zero) {
  if (zero) {
    flag = flag | 1;
  } else {
    flag = flag & (~1);
  }
}

constexpr void SetN(uint8_t& flag, bool negative) {
  if (negative) {
    flag = flag | 2;
  } else {
    flag = flag & (~2);
  }
}

constexpr void SetC(uint8_t& flag, bool carry) {
  if (carry) {
    flag = flag | 4;
  } else {
    flag = flag & (~4);
  }
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

constexpr uint32_t reladdr(const uint32_t v24bit) {
  return (v24bit >> 23) & 1 ? -((~(0xFF000000 | v24bit) + 1)/kWordSize)
                            : v24bit/kWordSize;
}

}  // namespace

CPU::CPU()
    : pc_(0), sp_(reg_[kRegCount-2]), fp_(reg_[kRegCount-1]), sflags_(0) {
  std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
}

void CPU::ConnectMemory(uint32_t* mem, uint32_t mem_size_bytes) {
  assert(mem != nullptr);
  mem_ = mem;
  mem_size_ = mem_size_bytes / kWordSize;

  std::memset(mem_, 0, mem_size_bytes);
  fp_ = sp_ = mem_size_;
}

void CPU::RegisterVideoDMA(VideoController* controller) {
  assert(controller != nullptr);
  controller->RegisterDMA(kVideoMemReg, kVideoMemStart, kFrameBufferW,
                          kFrameBufferH, 32, mem_);
}

void CPU::LoadProgram(uint32_t start, const std::vector<Word>& program) {
  start = start / kWordSize;
  assert(!program.empty());
  assert(program.size() + start < mem_size_);
  for (uint32_t idx = start, i = 0; i < program.size(); ++idx, i++) {
    mem_[idx] = program[i];
  }
}

void CPU::SetPC(uint32_t pc) {
  assert(pc % mem_size_ == 0);
  pc_ = pc / kWordSize;
  assert(pc_ < mem_size_);
}

const bool CPU::Step() {
  if (pc_ >= mem_size_) return false;
  const auto& word = mem_[pc_];
  switch (word & 0xFF) {  // first 8 bits define the instruction.
    case ISA::HALT:
      return false;
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
    case ISA::LOAD_IX:
      reg_[reg1(word)] = mem_[(reg_[reg2(word)] + v16bit(word))/kWordSize];
      break;
    case ISA::STOR_RR:
      mem_[reg_[reg1(word)]/kWordSize] = reg_[reg2(word)];
      break;
    case ISA::STOR_RI:
      mem_[((word >> 12) & 0xFFFFF)/kWordSize] = reg_[reg1(word)];
      break;
    case ISA::STOR_IX:
      mem_[(reg_[reg1(word)] + v16bit(word))/kWordSize] = reg_[reg2(word)];
      break;
    case ISA::ADD_RR: {
      const uint32_t v = reg_[reg2(word)] + reg_[reg3(word)];
      reg_[reg1(word)] = v;
      SetZ(sflags_, v == 0);
      SetN(sflags_, v >> 31 == 1);
      break;
    }
    case ISA::ADD_RI: {
      const uint32_t v = reg_[reg2(word)] + v16bit(word);
      reg_[reg1(word)] = v;
      SetZ(sflags_, v == 0);
      SetN(sflags_, v >> 31 == 1);
      break;
    }
    case ISA::SUB_RR: {
      const uint32_t op2 = (~reg_[reg3(word)] + 1);
      const uint32_t v = reg_[reg2(word)] + op2;
      reg_[reg1(word)] = v;
      SetZ(sflags_, v == 0);
      SetN(sflags_, v >> 31 == 1);
      break;
    }
    case ISA::SUB_RI: {
      const uint32_t op2 = (~v16bit(word) + 1);
      const uint32_t v = reg_[reg2(word)] + op2;
      reg_[reg1(word)] = v;
      SetZ(sflags_, v == 0);
      SetN(sflags_, v >> 31 == 1);
      break;
    }
    case ISA::JMP:
      pc_ = pc_ + reladdr(word >> 8) - 1;
      break;
    case ISA::JNE:
      if (!IsSetZ(sflags_)) pc_ = pc_ + reladdr(word >> 8) - 1;
      break;
    case ISA::JEQ:
      if (IsSetZ(sflags_)) pc_ = pc_ + reladdr(word >> 8) - 1;
      break;
    case ISA::JGT:
      if (!IsSetZ(sflags_) && !IsSetN(sflags_)) {
        pc_ = pc_ + reladdr(word >> 8) - 1;
      }
      break;
    case ISA::JGE:
      if (IsSetZ(sflags_) || !IsSetN(sflags_)) {
        pc_ = pc_ + reladdr(word >> 8) - 1;
      }
      break;
    case ISA::JLT:
      if (!IsSetZ(sflags_) && IsSetN(sflags_)) {
        pc_ = pc_ + reladdr(word >> 8) - 1;
      }
      break;
    case ISA::JLE:
      if (IsSetZ(sflags_) || IsSetN(sflags_)) {
        pc_ = pc_ + reladdr(word >> 8) - 1;
      }
      break;
    case ISA::CALL:
      mem_[--sp_] = pc_;
      mem_[--sp_] = fp_;
      fp_ = sp_;
      pc_ = pc_ + reladdr(word >> 8) - 1;
      break;
    case ISA::RET:
      sp_ = fp_;
      fp_ = mem_[sp_++];
      pc_ = mem_[sp_++];
      break;
    case ISA::AND_RR:
      reg_[reg1(word)] = reg_[reg2(word)] & reg_[reg3(word)];
      break;
    case ISA::AND_RI:
      reg_[reg1(word)] = reg_[reg2(word)] & ext16bit(v16bit(word));
      break;
    case ISA::ORR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] | reg_[reg3(word)];
      break;
    case ISA::ORR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] | ext16bit(v16bit(word));
      break;
    case ISA::XOR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] ^ reg_[reg3(word)];
      break;
    case ISA::XOR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] ^ ext16bit(v16bit(word));
      break;
    case ISA::LSL_RR:
      reg_[reg1(word)] = reg_[reg2(word)] << reg_[reg2(word)];
      break;
    case ISA::LSL_RI:
      reg_[reg1(word)] = reg_[reg2(word)] << v16bit(word);
      break;
    case ISA::LSR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] >> reg_[reg2(word)];
      break;
    case ISA::LSR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] >> v16bit(word);
      break;
    case ISA::ASR_RR:
      reg_[reg1(word)] = static_cast<int32_t>(reg_[reg2(word)]) >> reg_[reg3(word)];
      break;
    case ISA::ASR_RI:
      reg_[reg1(word)] = static_cast<int32_t>(reg_[reg2(word)]) >> v16bit(word);
      break;
    default:
      assert(false);
  }
  ++pc_;
  return true;
}

const std::string CPU::PrintRegisters(bool hex) {
  std::stringstream ss;

  ss << "[";
  for (uint32_t i = 0; i < kRegCount; ++i) {
    if (i != 0) ss << " ";
    const uint32_t r = (i >= 14) ? 4 *reg_[i] : reg_[i];
    if (hex) {
      ss << std::hex << "0x";
    } else {
      ss << std::dec;
    }
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
  assert(from < mem_size_);
  assert(to < mem_size_);

  std::stringstream ss;

  ss << "Memory from 0x" << std::hex << from << " to 0x" << std::hex << to
     << ":\n";
  for (uint32_t i = from; i <= to; i+=4) {
    ss << "0x" << std::hex << i << ": " << std::dec << mem_[i/kWordSize] << "\n";
  }
  return ss.str();
}

const std::string CPU::PrintStatusFlags() {
  std::stringstream ss;
  ss << "[Zero(" << IsSetZ(sflags_) << ") Neg(" << IsSetN(sflags_)
     << ") Carry(" << IsSetC(sflags_) << ")]\n";
  return ss.str();
}

}  // namespace gvm
