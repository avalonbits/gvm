#include "cpu.h"

#include <cassert>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "isa.h"

namespace gvm {

static const uint32_t kVideoMemReg = 0x00;
static const uint32_t kVideoMemStart = 0xFEE33C0;
static const uint32_t kVideoMemSizeWords = 720 * 405;
static const uint32_t kVideoMemEnd = kVideoMemStart + kVideoMemSizeWords;
static const int kFrameBufferW = 720;
static const int kFrameBufferH = 405;

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

constexpr uint32_t reladdr(const uint32_t v24bit) {
  return (v24bit >> 23) & 1 ? -((~(0xFF000000 | v24bit) + 1)/kWordSize)
                            : v24bit/kWordSize;
}

constexpr uint32_t reladdr20(const uint32_t v) {
  const uint32_t v20bit = v >> 12;
  return (v20bit >> 19) & 1 ? -((~(0xFFF00000 | v20bit) + 1)/kWordSize)
                            : v20bit/kWordSize;
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
  controller->RegisterDMA(kVideoMemReg, kVideoMemStart / kWordSize, kFrameBufferW,
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
  assert(pc % kWordSize == 0);
  pc_ = pc / kWordSize;
  assert(pc_ < mem_size_);
}

uint32_t CPU::Run(const bool debug) {
  uint32_t i = 1;
  if (debug) std::cerr << PrintRegisters(/*hex=*/true) << std::endl;
  for (;;++i,++pc_) {
    if (debug) {
      std::cerr << PrintRegisters(/*hex=*/true) << std::endl;
      getchar();
    }

  if (pc_ >= mem_size_) break;
  const auto& word = mem_[pc_];
  if (debug) {
    std::cerr << "0x" << std::hex << pc_ << ": " << std::dec << PrintInstruction(word) << std::endl;
  }
  switch (word & 0xFF) {  // first 8 bits define the instruction.
    case ISA::HALT:
      return i;
      continue;
    case ISA::NOP:
      continue;
    case ISA::MOV_RR:
      reg_[reg1(word)] = reg_[reg2(word)];
      continue;
    case ISA::MOV_RI: {
      uint32_t v = (word >> 12);
      if (((v >> 19) & 1) == 1) v = 0xFFF00000 | v;
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::LOAD_RR:
      reg_[reg1(word)] = mem_[reg_[reg2(word)]/kWordSize];
      continue;
    case ISA::LOAD_RI:
      reg_[reg1(word)] = mem_[((word >> 12) & 0xFFFFF)/kWordSize];
      continue;
    case ISA::LOAD_IX:
      reg_[reg1(word)] = mem_[(reg_[reg2(word)] + v16bit(word))/kWordSize];
      continue;
    case ISA::STOR_RR:
      mem_[reg_[reg1(word)]/kWordSize] = reg_[reg2(word)];
      continue;
    case ISA::STOR_RI:
      mem_[((word >> 12) & 0xFFFFF)/kWordSize] = reg_[reg1(word)];
      continue;
    case ISA::STOR_IX:
      mem_[(reg_[reg1(word)] + v16bit(word))/kWordSize] = reg_[reg2(word)];
      continue;
    case ISA::ADD_RR: {
      const uint32_t v = reg_[reg2(word)] + reg_[reg3(word)];
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::ADD_RI: {
      const uint32_t v = reg_[reg2(word)] + v16bit(word);
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::SUB_RR: {
      const uint32_t op2 = (~reg_[reg3(word)] + 1);
      const uint32_t v = reg_[reg2(word)] + op2;
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::SUB_RI: {
      const uint32_t op2 = (~v16bit(word) + 1);
      const uint32_t v = reg_[reg2(word)] + op2;
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::JMP:
      pc_ = pc_ + reladdr(word >> 8) - 1;
      continue;
    case ISA::JNE:
      if (reg_[reg1(word)] != 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::JEQ:
      if (reg_[reg1(word)] == 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::JGT:
      if (reg_[reg1(word)] > 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::JGE:
      if (reg_[reg1(word)] >= 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::JLT:
      if (reg_[reg1(word)] < 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::JLE:
      if (reg_[reg1(word)] <= 0) pc_ = pc_ + reladdr20(word) - 1;
      continue;
    case ISA::CALLI:
      mem_[--sp_] = pc_;
      mem_[--sp_] = fp_;
      fp_ = sp_;
      pc_ = pc_ + reladdr(word >> 8) - 1;
      continue;
    case ISA::CALLR:
      mem_[--sp_] = pc_;
      mem_[--sp_] = fp_;
      fp_ = sp_;
      pc_ = reg_[reg1(word)]/kWordSize - 1;
      continue;
    case ISA::RET:
      sp_ = fp_;
      fp_ = mem_[sp_++];
      pc_ = mem_[sp_++];
      continue;
    case ISA::AND_RR: {
      const uint32_t v = reg_[reg2(word)] & reg_[reg3(word)];
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::AND_RI: {
      const uint32_t v = (reg_[reg2(word)]) & (v16bit(word));
      reg_[reg1(word)] = v;
      continue;
    }
    case ISA::ORR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] | reg_[reg3(word)];
      continue;
    case ISA::ORR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] | v16bit(word);
      continue;
    case ISA::XOR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] ^ reg_[reg3(word)];
      continue;
    case ISA::XOR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] ^ v16bit(word);
      continue;
    case ISA::LSL_RR:
      reg_[reg1(word)] = reg_[reg2(word)] << reg_[reg3(word)];
      continue;
    case ISA::LSL_RI:
      reg_[reg1(word)] = reg_[reg2(word)] << v16bit(word);
      continue;
    case ISA::LSR_RR:
      reg_[reg1(word)] = reg_[reg2(word)] >> reg_[reg3(word)];
      continue;
    case ISA::LSR_RI:
      reg_[reg1(word)] = reg_[reg2(word)] >> v16bit(word);
      continue;
    case ISA::ASR_RR:
      reg_[reg1(word)] = static_cast<int32_t>(reg_[reg2(word)]) >> reg_[reg3(word)];
      continue;
    case ISA::ASR_RI:
      reg_[reg1(word)] = static_cast<int32_t>(reg_[reg2(word)]) >> v16bit(word);
      continue;
    default:
      assert(false);
  }
  }
  return i;
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

std::string CPU::PrintInstruction(const Word word) {
  std::ostringstream ss;

  switch (word & 0xFF) {  // first 8 bits define the instruction.
    case ISA::HALT:
      return "halt";
      break;
    case ISA::NOP:
      return "nop";
      break;
    case ISA::MOV_RR:
      ss << "mov r" << reg1(word) << ", r" << reg2(word);
      break;
    case ISA::MOV_RI: {
      uint32_t v = (word >> 12);
      if (((v >> 19) & 1) == 1) v = 0xFFF00000 | v;
      ss << "mov r" << reg1(word) << ", 0x" << std::hex << v;
      break;
    }
    case ISA::LOAD_RR:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << "]";
      break;
    case ISA::LOAD_RI:
      ss << "load r" << reg1(word) << ", [0x" << std::hex << ((word >> 12) & 0xFFFFF) << "]";
      break;
    case ISA::LOAD_IX:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << ", 0x" << std::hex << v16bit(word) << "]";
      break;
    case ISA::STOR_RR:
      ss << "stor [r" << reg1(word) << "], r" << reg2(word);
      break;
    case ISA::STOR_RI:
      ss << "stor [0x" << std::hex << ((word >> 12) & 0xFFFFF) << "], r" << std::dec << reg1(word);
      break;
    case ISA::STOR_IX:
      ss << "stor [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word);
      break;
    case ISA::ADD_RR:
      ss << "add r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::ADD_RI:
      ss << "add r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::SUB_RR:
      ss << "sub r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::SUB_RI:
      ss << "sub r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::JMP:
      ss << "jmp 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JNE:
      ss << "jne 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JEQ:
      ss << "jeq 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JGT:
      ss << "jgt 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JGE:
      ss << "jge 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JLT:
      ss << "jlt 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::JLE:
      ss << "jle 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::CALLI:
      ss << "call 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr(word >> 8)));
      break;
    case ISA::CALLR:
      ss << "call [r" << reg1(word) << "]";
      break;
    case ISA::RET:
      return "ret";
    case ISA::AND_RR:
      ss << "and r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::AND_RI:
      ss << "and r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::ORR_RR:
      ss << "orr r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::ORR_RI:
      ss << "orr r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::XOR_RR:
      ss << "xor r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::XOR_RI:
      ss << "xor r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::LSL_RR:
      ss << "lsl r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::LSL_RI:
      ss << "lsl r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::LSR_RR:
      ss << "lsr r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::LSR_RI:
      ss << "lsr r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::ASR_RR:
      ss << "asr r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::ASR_RI:
      ss << "asr r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    default:
      assert(false);
  }
  return ss.str();
}

}  // namespace gvm
