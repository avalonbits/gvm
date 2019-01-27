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

namespace {
constexpr uint32_t regv(const uint32_t idx, const uint32_t pc, uint32_t* regs) {
  if (idx < 29) return regs[idx];
  if (idx == 29) return  pc << 2;
  return regs[idx] << 2;
}
constexpr uint32_t reg1(uint32_t word) {
  return (word >> 6) & 0x1F;
}
constexpr uint32_t reg2(uint32_t word) {
  return (word >> 11) & 0x1F;
}
constexpr uint32_t reg3(uint32_t word) {
  return (word >> 16) & 0x1F;
}
constexpr uint32_t reg4(uint32_t word) {
  return (word >> 21) & 0x1F;
}
constexpr uint32_t v16bit(uint32_t word) {
  return (word >> 16) & 0xFFFF;
}
constexpr uint32_t ext16bit(uint32_t word) {
  return (0x00008000 & word) ? (0xFFFF0000 | word) : word;
}

constexpr uint32_t reladdr26(const uint32_t v26bit) {
  return (v26bit >> 25) & 1 ? -((~(0xFC000000 | v26bit) + 1)/kWordSize)
                            : v26bit/kWordSize;
}

constexpr uint32_t reladdr21(const uint32_t v) {
  const uint32_t v21bit = v >> 11;
  return (v21bit >> 20) & 1 ? -((~(0xFFE00000 | v21bit) + 1)/kWordSize)
                            : v21bit/kWordSize;
}

}  // namespace

CPU::CPU()
    : pc_(reg_[kRegCount-3]), sp_(reg_[kRegCount-2]), fp_(reg_[kRegCount-1]) {
  std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
}

void CPU::ConnectMemory(uint32_t* mem, uint32_t mem_size_bytes) {
  assert(mem != nullptr);
  mem_ = mem;
  mem_size_ = mem_size_bytes / kWordSize;
  std::memset(mem_, 0, mem_size_bytes);
  fp_ = sp_ = mem_size_;
}

void CPU::LoadProgram(const std::map<uint32_t, std::vector<Word>>& program) {
  for (const auto& kv : program) {
    const auto& start = kv.first / kWordSize;
    const auto& words = kv.second;
    assert(!words.empty());
    assert(words.size() + start < mem_size_);
    for (uint32_t idx = start, i = 0; i < words.size(); ++idx, ++i) {
      mem_[idx] = words[i];
    }
  }
}

void CPU::SetPC(uint32_t pc) {
  assert(pc % kWordSize == 0);
  pc_ = pc / kWordSize;
  assert(pc_ < mem_size_);
}

uint32_t CPU::Run() {
  static void* opcodes[] = {
    &&NOP, &&HALT, &&MOV_RR, &&MOV_RI, &&LOAD_RR, &&LOAD_RI, &&LOAD_IX,
    &&STOR_RR, &&STOR_RI, &&STOR_IX, &&ADD_RR, &&ADD_RI, &&SUB_RR,
    &&SUB_RI, &&JMP, &&JNE, &&JEQ, &&JGT, &&JGE, &&JLT, &&JLE, &&CALLI,
    &&CALLR, &&RET, &&AND_RR, &&AND_RI, &&ORR_RR, &&ORR_RI, &&XOR_RR,
    &&XOR_RI, &&LSL_RR, &&LSL_RI, &&LSR_RR, &&LSR_RI, &&ASR_RR, &&ASR_RI,
    &&MUL_RR, &&MUL_RI, &&DIV_RR, &&DIV_RI, &&MULL_RR
  };
  register uint32_t pc = pc_-1;
  register uint32_t i = 0;
  register uint32_t word = 0;
  uint32_t interrupt = 1;


#define code_dispatch() \
  if (interrupt > 0) {\
    goto INTERRUPT_SERVICE;\
  } else {\
    ++pc;\
    ++i;\
    interrupt = 1;\
    word =  mem_[pc];\
    goto *opcodes[word&0x3F];\
  }

#ifdef DEBUG_DISPATCH
#define DISPATCH() \
    pc_ = pc * 4; \
    std::cerr << "0x" << std::hex << pc_ << ": " << std::dec \
              << PrintInstruction(word) << std::endl; \
      std::cerr << PrintRegisters(true) << std::endl;\
      getchar(); \
    code_dispatch()
#else
#define DISPATCH() code_dispatch()
#endif

  DISPATCH();
  NOP:
      DISPATCH();
  HALT: {
    return i;
  }
  MOV_RR: {
      const register int32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  MOV_RI: {
      const register uint32_t idx = reg1(word);
      register int32_t v = (word >> 11);
      if (((v >> 20) & 1) == 1) v = 0xFFE00000 | v;
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LOAD_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = mem_[regv(reg2(word), pc, reg_)/kWordSize];
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LOAD_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = mem_[((word >> 11) & 0x1FFFF)/kWordSize];
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LOAD_IX: {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          mem_[(regv(reg2(word), pc, reg_) + v16bit(word))/kWordSize];
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  STOR_RR:
      mem_[regv(reg1(word), pc, reg_)/kWordSize] = regv(reg2(word), pc, reg_);
      DISPATCH();
  STOR_RI:
      mem_[((word >> 11) & 0x1FFFFF)/kWordSize] = reg_[reg1(word)];
      DISPATCH();
  STOR_IX:
      mem_[(regv(reg1(word), pc, reg_) + v16bit(word))/kWordSize] =
          regv(reg2(word), pc, reg_);
      DISPATCH();
  ADD_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc ,reg_) + regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  ADD_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) + v16bit(word);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  SUB_RR: {
      const register uint32_t idx = reg1(word);
      const register uint32_t op2 = (~regv(reg3(word), pc, reg_) + 1);
      const register int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  SUB_RI: {
      const register uint32_t op2 = (~v16bit(word) + 1);
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  JMP:
      pc = pc + reladdr26(word >> 6) - 1;
      DISPATCH();
  JNE:
      if (reg_[reg1(word)] != 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  JEQ:
      if (reg_[reg1(word)] == 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  JGT:
      if (reg_[reg1(word)] > 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  JGE:
      if (reg_[reg1(word)] >= 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  JLT:
      if (reg_[reg1(word)] < 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  JLE:
      if (reg_[reg1(word)] <= 0) pc = pc + reladdr21(word) - 1;
      DISPATCH();
  CALLI:
      mem_[--sp_] = pc;
      mem_[--sp_] = fp_;
      fp_ = sp_;
      pc = pc + reladdr26(word >> 6) - 1;
      DISPATCH();
  CALLR:
      mem_[--sp_] = pc;
      mem_[--sp_] = fp_;
      fp_ = sp_;
      pc = reg_[reg1(word)]/kWordSize - 1;
      DISPATCH();
  RET:
      sp_ = fp_;
      fp_ = mem_[sp_++];
      pc = mem_[sp_++];
      DISPATCH();
  AND_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) & regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  AND_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) & v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  ORR_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) | regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  ORR_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) | v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  XOR_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) ^ regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  XOR_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) ^ v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LSL_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) << regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LSL_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) << v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LSR_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          (regv(reg2(word), pc, reg_) >> regv(reg3(word), pc, reg_));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  LSR_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) >> v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  ASR_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          static_cast<int32_t>((regv(reg2(word), pc, reg_)) >>
              regv(reg3(word), pc, reg_));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  ASR_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          (static_cast<int32_t>(regv(reg2(word), pc, reg_)) >> v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  MUL_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) * regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  MUL_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) * v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  DIV_RR: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) / regv(reg3(word), pc, reg_);
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  DIV_RI: {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) / v16bit(word));
      reg_[idx] = (idx >= 29) ? v / kWordSize : v;
      DISPATCH();
  }
  MULL_RR: {
      const register uint32_t idxH = reg1(word);
      const register uint32_t idxL = reg2(word);
      const register int64_t v = regv(reg3(word), pc, reg_) * regv(reg4(word), pc, reg_);
      reg_[idxL] = (idxL >= 29) ? (v & 0xFFFFFFFF) / kWordSize : (v & 0xFFFFFFFF);
      const register int32_t vH = (v >> 32);
      reg_[idxH] = (idxH >= 29) ? vH / kWordSize : vH;
      DISPATCH();
  }

  INTERRUPT_SERVICE: {
    interrupt = 0;
    DISPATCH();
  }
}

const std::string CPU::PrintRegisters(bool hex) {
  std::stringstream ss;

  ss << "[";
  for (uint32_t i = 0; i < kRegCount; ++i) {
    if (i != 0) ss << " ";
    const uint32_t r = (i >= 29) ? 4 *reg_[i] : reg_[i];
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

  switch (word & 0x3F) {  // first 8 bits define the instruction.
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
      uint32_t v = (word >> 11);
      if (((v >> 20) & 1) == 1) v = 0xFFE00000 | v;
      ss << "mov r" << reg1(word) << ", 0x" << std::hex << v;
      break;
    }
    case ISA::LOAD_RR:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << "]";
      break;
    case ISA::LOAD_RI:
      ss << "load r" << reg1(word) << ", [0x" << std::hex << ((word >> 11) & 0x1FFFFF) << "]";
      break;
    case ISA::LOAD_IX:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << ", 0x" << std::hex << v16bit(word) << "]";
      break;
    case ISA::STOR_RR:
      ss << "stor [r" << reg1(word) << "], r" << reg2(word);
      break;
    case ISA::STOR_RI:
      ss << "stor [0x" << std::hex << ((word >> 11) & 0x1FFFFF) << "], r" << std::dec << reg1(word);
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
      ss << "jmp 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JNE:
      ss << "jne 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JEQ:
      ss << "jeq 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JGT:
      ss << "jgt 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JGE:
      ss << "jge 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JLT:
      ss << "jlt 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::JLE:
      ss << "jle 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
      break;
    case ISA::CALLI:
      ss << "call 0x" << std::hex << (pc_ + static_cast<int32_t>(reladdr26(word >> 6)));
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
