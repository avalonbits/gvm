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
constexpr uint32_t m2w(uint32_t idx) {
  return idx / kWordSize;
}
constexpr uint32_t regv(const uint32_t idx, const uint32_t pc, uint32_t* regs) {
  if (idx != 29 && idx != 28) return regs[idx];
  if (idx == 29) return pc;
  return 0;
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
constexpr uint32_t v11bit(uint32_t word) {
  return (word >> 21) & 0x7FF;
}

constexpr uint32_t ext16bit(uint32_t word) {
  word = v16bit(word);
  return (0x00008000 & word) ? (0xFFFF0000 | word) : word;
}
constexpr uint32_t ext11bit(uint32_t word) {
  word = v11bit(word);
  return (0x00000400 & word) ? (0xFFFFF800 | word) : word;
}

constexpr uint32_t reladdr26(const uint32_t v26bit) {
  return (v26bit >> 25) & 1 ? -(~(0xFC000000 | v26bit) + 1)
                            : v26bit;
}

constexpr uint32_t reladdr21(const uint32_t v) {
  const uint32_t v21bit = v >> 11;
  return (v21bit >> 20) & 1 ? -(~(0xFFE00000 | v21bit) + 1)
                            : v21bit;
}

}  // namespace

CPU::CPU()
    : pc_(reg_[kRegCount-3]), sp_(reg_[kRegCount-2]), fp_(reg_[kRegCount-1]),
      op_count_(0), mask_interrupt_(false), interrupt_(0) {
  std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
  SetupHandlers();
}

void CPU::ConnectMemory(uint32_t* mem, uint32_t mem_size_bytes, uint32_t user_ram_limit) {
  assert(mem != nullptr);
  mem_ = mem;
  mem_size_ = mem_size_bytes;
  user_ram_limit_ = user_ram_limit;
  std::memset(mem_, 0, m2w(mem_size_bytes));
  fp_ = sp_ = user_ram_limit_;
}

void CPU::SetPC(uint32_t pc) {
  assert(pc % kWordSize == 0);
  pc_ = pc;
  assert(pc_ < mem_size_);
}

uint32_t CPU::PowerOn() {
  Reset();
#ifdef CPU_HANDLERS
  RunHandlers();
#endif
#ifndef CPU_HANDLERS
  Run();
#endif
  return op_count_;
}

uint32_t CPU::Reset() {
  const uint32_t op_count = op_count_;
  interrupt_ = 1;  // Mask out all interrupts and set bit 0 to 1, signaling reset.
  interrupt_event_.notify_one();
  return op_count;
}

void CPU::Timer() {
  if (mask_interrupt_) return;
  interrupt_ |= 0x02;
  interrupt_event_.notify_all();
}

void CPU::Input() {
  if (mask_interrupt_) return;
  interrupt_ |= 0x04;
  interrupt_event_.notify_all();
}

void CPU::RecurringTimer() {
  if (mask_interrupt_) return;
  interrupt_ |= 0x08;
  interrupt_event_.notify_all();
}

void CPU::Timer2() {
  if (mask_interrupt_) return;
  interrupt_ |= 0x10;
  interrupt_event_.notify_all();
}

void CPU::RecurringTimer2() {
  if (mask_interrupt_) return;
  interrupt_ |= 0x20;
  interrupt_event_.notify_all();
}

void CPU::Run() {
  static void* opcodes[] = {
    &&NOP, &&HALT, &&LOAD_RI, &&LOAD_IX, &&LOAD_IXR, &&LOAD_PI, &&LOAD_IP,
    &&LDP_PI, &&LDP_IP, &&STOR_RI, &&STOR_IX, &&STOR_PI, &&STOR_IP, &&STP_PI,
    &&STP_IP, &&ADD_RR, &&ADD_RI, &&SUB_RR, &&SUB_RI, &&JMP, &&JNE,
    &&JEQ, &&JGT, &&JGE, &&JLT, &&JLE, &&CALLI, &&CALLR, &&RET, &&AND_RR,
    &&AND_RI, &&ORR_RR, &&ORR_RI, &&XOR_RR, &&XOR_RI, &&LSL_RR, &&LSL_RI,
    &&LSR_RR, &&LSR_RI, &&ASR_RR, &&ASR_RI, &&MUL_RR, &&MUL_RI, &&DIV_RR,
    &&DIV_RI, &&MULL_RR, &&WFI
  };
  uint32_t pc = pc_-4;
  uint32_t word = 0;

#define interrupt_dispatch() \
  if (interrupt_ == 0) {\
    pc += 4;\
    word =  mem_[m2w(pc)];\
    ++op_count_;\
    goto *opcodes[word&0x3F];\
  }\
  goto INTERRUPT_SERVICE

#ifdef DEBUG_DISPATCH
#define DISPATCH() \
    pc_ = pc; \
    std::cerr << PrintInstruction(word) << std::endl; \
    std::cerr << PrintRegisters(true) << std::endl;\
    interrupt_dispatch()
#else
#define DISPATCH() interrupt_dispatch()
#endif

#define VSIG(addr) \
  if (addr == vram_reg_) {\
    video_signal_->send();\
    video_signal_->recv();\
  }

#define TIMER_READ(addr, v, fallback) \
  v = fallback; \
  if (addr == timer_reg_) { \
    v = timer_signal_->Elapsed();\
  }\

#define TIMER_WRITE(addr, v) \
  if (addr >= oneshot_reg_) { \
    if (addr == oneshot_reg_) { \
      timer_signal_->OneShot(v); \
    } else if (addr == recurring_reg_) { \
      timer_signal_->Recurring(v); \
    } else if (addr == oneshot2_reg_) { \
      timer2_signal_->OneShot(v); \
    } else if (addr == recurring2_reg_) { \
      timer2_signal_->Recurring(v); \
    } \
  }

  DISPATCH();
  NOP:
      DISPATCH();
  HALT: {
    return;
  }
  LOAD_RI: {
      const uint32_t idx = reg1(word);
      const uint32_t addr = (word >> 11) & 0x1FFFFF;
      int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  }
  LOAD_IX: {
      const uint32_t idx = reg1(word);
      const uint32_t addr = regv(reg2(word), pc, reg_) + ext16bit(word);
      int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  }
  LOAD_IXR: {
      const uint32_t idx = reg1(word);
      const uint32_t addr =
         regv(reg2(word), pc, reg_) + regv(reg3(word), pc, reg_);
      int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  }
  LOAD_PI: {
      const uint32_t idx = reg1(word);
      const uint32_t idx2 = reg2(word);
      const uint32_t next = regv(idx2, pc, reg_) + ext16bit(word);
      int32_t v;
      TIMER_READ(next, v, mem_[m2w(next)]);
      reg_[idx] = v;
      reg_[idx2] = next;
      DISPATCH();
  }
  LOAD_IP: {
      const uint32_t idx = reg1(word);
      const uint32_t idx2 = reg2(word);
      const uint32_t cur = regv(idx2, pc, reg_);
      const uint32_t next = cur + ext16bit(word);
      int32_t v;
      TIMER_READ(cur, v, mem_[m2w(cur)]);
      reg_[idx] = v;
      reg_[idx2] = next;
      DISPATCH();
  }
  LDP_PI: {
      const uint32_t src = reg3(word);
      const uint32_t dest1 = reg1(word);
      const uint32_t dest2 = reg2(word);
      const uint32_t next = regv(src, pc, reg_) + ext11bit(word);
      int32_t v;
      TIMER_READ(next, v, mem_[m2w(next)]);
      reg_[dest1] = v;
      TIMER_READ(next+4, v, mem_[m2w(next+4)]);
      reg_[dest2] = v;
      reg_[src] = next;
      DISPATCH();
  }
  LDP_IP: {
      const uint32_t src = reg3(word);
      const uint32_t dest1 = reg1(word);
      const uint32_t dest2 = reg2(word);
      const uint32_t cur = regv(src, pc, reg_);
      const uint32_t next = cur + ext11bit(word);
      int32_t v;
      TIMER_READ(cur, v, mem_[m2w(cur)]);
      reg_[dest1] = v;
      TIMER_READ(cur+4, v, mem_[m2w(cur+4)]);
      reg_[dest2] = v;
      reg_[src] = next;
      DISPATCH();
  }
  STOR_RI: {
      const uint32_t addr = (word >> 11) & 0x1FFFFF;
      const auto v = mem_[m2w(addr)] = regv(reg1(word), pc, reg_);
      VSIG(addr);
      TIMER_WRITE(addr, v);
      DISPATCH();
  }
  STOR_IX: {
      const uint32_t addr = regv(reg1(word), pc, reg_) + ext16bit(word);
      const auto v = mem_[m2w(addr)] = regv(reg2(word), pc, reg_);
      VSIG(addr);
      TIMER_WRITE(addr, v);
      DISPATCH();
  }
  STOR_PI: {
      const uint32_t idx = reg1(word);
      const uint32_t next = regv(idx, pc, reg_) + ext16bit(word);
      const auto v = mem_[m2w(next)] = regv(reg2(word), pc, reg_);
      reg_[idx] = next;
      VSIG(next);
      TIMER_WRITE(next, v);
      DISPATCH();
  }
  STOR_IP: {
      const uint32_t idx = reg1(word);
      const uint32_t cur = regv(idx, pc, reg_);
      const uint32_t next = cur + ext16bit(word);
      const auto v = mem_[m2w(cur)] = regv(reg2(word), pc, reg_);
      reg_[idx] = next;
      VSIG(cur);
      TIMER_WRITE(cur, v);
      DISPATCH();
  }
  STP_PI: {
      const uint32_t dest = reg1(word);
      const uint32_t next = regv(dest, pc, reg_) + ext11bit(word);
      auto v = mem_[m2w(next)] = regv(reg2(word), pc, reg_);
      VSIG(next);
      TIMER_WRITE(next, v);
      v = mem_[m2w(next+4)] = regv(reg3(word), pc, reg_);
      VSIG(next+4);
      TIMER_WRITE(next+4, v);
      reg_[dest] = next;
      DISPATCH();
  }
  STP_IP: {
      const uint32_t dest = reg1(word);
      const uint32_t cur = regv(dest, pc, reg_);
      const uint32_t next = cur + ext11bit(word);
      auto v = mem_[m2w(cur)] = regv(reg2(word), pc, reg_);
      TIMER_WRITE(cur, v);
      VSIG(cur);
      v = mem_[m2w(cur+4)] = regv(reg3(word), pc, reg_);
      TIMER_WRITE(cur+4, v);
      VSIG(cur+4);
      reg_[dest] = next;
      DISPATCH();
  }
  ADD_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc ,reg_) + regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  ADD_RI: {
      const uint32_t idx = reg1(word);
      reg_[idx] = static_cast<int32_t>(regv(reg2(word), pc, reg_) + ext16bit(word));
      DISPATCH();
  }
  SUB_RR: {
      const uint32_t idx = reg1(word);
      const uint32_t op2 = (~regv(reg3(word), pc, reg_) + 1);
      const int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = v;
      DISPATCH();
  }
  SUB_RI: {
      const uint32_t op2 = (~ext16bit(word) + 1);
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = v;
      DISPATCH();
  }
  JMP:
      pc = pc + reladdr26(word >> 6) - 4;
      DISPATCH();
  JNE:
      if (reg_[reg1(word)] != 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  JEQ:
      if (reg_[reg1(word)] == 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  JGT:
      if (static_cast<int32_t>(reg_[reg1(word)]) > 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  JGE:
      if (static_cast<int32_t>(reg_[reg1(word)]) >= 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  JLT:
      if (static_cast<int32_t>(reg_[reg1(word)]) < 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  JLE:
      if (static_cast<int32_t>(reg_[reg1(word)]) <= 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  CALLI:
      sp_ -= 4;
      mem_[m2w(sp_)] = pc;
      sp_ -= 4;
      mem_[m2w(sp_)] = fp_;
      fp_ = sp_;
      pc = pc + reladdr26(word >> 6) - 4;
      DISPATCH();
  CALLR:
      sp_ -= 4;
      mem_[m2w(sp_)] = pc;
      sp_ -= 4;
      mem_[m2w(sp_)] = fp_;
      fp_ = sp_;
      pc = reg_[reg1(word)] - 4;
      DISPATCH();
  RET:
      sp_ = fp_;
      fp_ = mem_[m2w(sp_)];
      sp_ += 4;
      pc = mem_[m2w(sp_)];
      sp_ += 4;
      mask_interrupt_ = false;
      DISPATCH();
  AND_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) & regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  AND_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) & v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  ORR_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) | regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  ORR_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) | v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  XOR_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) ^ regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  XOR_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) ^ v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  LSL_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) << regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  LSL_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) << v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  LSR_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v =
          (regv(reg2(word), pc, reg_) >> regv(reg3(word), pc, reg_));
      reg_[idx] = v;
      DISPATCH();
  }
  LSR_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) >> v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  ASR_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v =
          static_cast<int32_t>((regv(reg2(word), pc, reg_)) >>
              regv(reg3(word), pc, reg_));
      reg_[idx] = v;
      DISPATCH();
  }
  ASR_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v =
          (static_cast<int32_t>(regv(reg2(word), pc, reg_)) >> v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  MUL_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) * regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  MUL_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) * ext16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  DIV_RR: {
      const uint32_t idx = reg1(word);
      const int32_t v = regv(reg2(word), pc, reg_) / regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  }
  DIV_RI: {
      const uint32_t idx = reg1(word);
      const int32_t v = (regv(reg2(word), pc, reg_) / ext16bit(word));
      reg_[idx] = v;
      DISPATCH();
  }
  MULL_RR: {
      const uint32_t idxH = reg1(word);
      const uint32_t idxL = reg2(word);
      const int64_t v = regv(reg3(word), pc, reg_) * regv(reg4(word), pc, reg_);
      reg_[idxL] = (v & 0xFFFFFFFF);
      const int32_t vH = (v >> 32);
      reg_[idxH] = vH;
      DISPATCH();
  }
  WFI: {
    // Wait on mutex.
    std::unique_lock<std::mutex> ul(interrupt_mutex_);
    interrupt_event_.wait(ul, [this]{return interrupt_ != 0;});
    DISPATCH();
    return;
  }

  INTERRUPT_SERVICE: {
    // If reset is set, we ignore every other signal and reset the cpu.
    if (interrupt_ & 0x01) {
      interrupt_ = 0;
      // We zero out all registers and setup pc, sp and fp accordingly.
      std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
      fp_ = sp_ = user_ram_limit_;
      pc = pc_-4;
    } else {
      mask_interrupt_ = true;
      sp_ -= 4;
      mem_[m2w(sp_)] = pc;
      sp_ -= 4;
      mem_[m2w(sp_)] = fp_;
      fp_ = sp_;

      // Process signals in bit order. Lower bits have higher priority than higher bits.
      if (interrupt_ & 0x02) {
        // Timer interrupt.
        pc = 0x0;  // Set to 0 because it will be incremented to addr 0x04 on DISPATCH.
        interrupt_ &= ~0x02;
      } else if (interrupt_ & 0x04) {
        // Input interrupt.
        pc = 0x04;  // Set to 0x04 because it will be incremented to addr 0x08 on DISPATCH.
        interrupt_ &= ~0x04;
      } else if (interrupt_ & 0x08) {
        pc = 0x08;  // Set to 0x08 because it will be incremented to addr 0x0c on DISPATCH.
        interrupt_ &= ~0x08;
      } else if (interrupt_ & 0x10) {
        pc = 0x0c;  // Set to 0x0c because it will be incremented to addr 0x10 on DISPATCH.
        interrupt_ &= ~0x10;
      } else if (interrupt_ & 0x20) {
        pc = 0x10;  // Set to 0x10 because it will be incremented to addr 0x14 on DISPATCH.
        interrupt_ &= ~0x20;
      }
    }
    DISPATCH();
  }
}

void CPU::InterruptService(uint32_t& pc) {
  // If reset is set, we ignore every other signal and reset the cpu.
  if (interrupt_ & 0x01) {
    interrupt_ = 0;
    // We zero out all registers and setup pc, sp and fp accordingly.
    std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
    fp_ = sp_ = user_ram_limit_;
    pc = pc_;
  } else {
    mask_interrupt_ = true;
    sp_ -= 4;
    mem_[m2w(sp_)] = pc-4;
    sp_ -= 4;
    mem_[m2w(sp_)] = fp_;
    fp_ = sp_;

    // Process signals in bit order. Lower bits have higher priority than higher bits.
    if (interrupt_ & 0x02) {
      // Timer interrupt.
      pc = 0x4;
      interrupt_ &= ~0x02;
    } else if (interrupt_ & 0x04) {
      // Input interrupt.
      pc = 0x08;
      interrupt_ &= ~0x04;
    } else if (interrupt_ & 0x08) {
      pc = 0x0c;
      interrupt_ &= ~0x08;
    } else if (interrupt_ & 0x10) {
      pc = 0x10;
      interrupt_ &= ~0x10;
    } else if (interrupt_ & 0x20) {
      pc = 0x14;
      interrupt_ &= ~0x20;
    }
  }
}


void CPU::RunHandlers() {
  uint32_t pc = pc_;
  bool halt = false;
  uint32_t word = 0;
  do {
#ifdef DEBUG_DISPATCH
    pc_ = pc-4;
    std::cerr << PrintInstruction(word) << std::endl;
    std::cerr << PrintRegisters(true) << std::endl;
#endif 
    if (interrupt_ != 0) { InterruptService(pc); }
    word =  mem_[m2w(pc)];
    ++op_count_;
    handlers_[word&0x3F](word, pc, halt);
  } while (halt == false);
}

void CPU::SetupHandlers() {
#define REG_HANDLER(instr, body) \
  handlers_[ISA::instr] = [this](uint32_t word, uint32_t& pc, bool& halt) body

#ifdef DISPATCH
#undef DISPATCH
#endif

#define DISPATCH() pc += 4

  REG_HANDLER(NOP, { pc += 4; });
  REG_HANDLER(HALT, { halt = true; });
  REG_HANDLER(LOAD_RI, {
      const register uint32_t idx = reg1(word);
      const register uint32_t addr = (word >> 11) & 0x1FFFFF;
      register int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  });


  REG_HANDLER(LOAD_IX, {
      const register uint32_t idx = reg1(word);
      const register uint32_t addr = regv(reg2(word), pc, reg_) + ext16bit(word);
      register int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LOAD_IXR, {
      const register uint32_t idx = reg1(word);
      const register uint32_t addr =
         regv(reg2(word), pc, reg_) + regv(reg3(word), pc, reg_);
      register int32_t v;
      TIMER_READ(addr, v, mem_[m2w(addr)]);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LOAD_PI, {
      const register uint32_t idx = reg1(word);
      const register uint32_t idx2 = reg2(word);
      const register uint32_t next = regv(idx2, pc, reg_) + ext16bit(word);
      register int32_t v;
      TIMER_READ(next, v, mem_[m2w(next)]);
      reg_[idx] = v;
      reg_[idx2] = next;
      DISPATCH();
  });

  REG_HANDLER(LOAD_IP, {
      const register uint32_t idx = reg1(word);
      const register uint32_t idx2 = reg2(word);
      const register uint32_t cur = regv(idx2, pc, reg_);
      const register uint32_t next = cur + ext16bit(word);
      register int32_t v;
      TIMER_READ(cur, v, mem_[m2w(cur)]);
      reg_[idx] = v;
      reg_[idx2] = next;
      DISPATCH();
  });

  REG_HANDLER(LDP_PI, {
      const register uint32_t dest1 = reg1(word);
      const register uint32_t dest2 = reg2(word);
      const register uint32_t src = reg3(word);
      const register uint32_t next = regv(src, pc, reg_) + ext11bit(word);
      register int32_t v;
      TIMER_READ(next, v, mem_[m2w(next)]);
      reg_[dest1] = v;
      TIMER_READ(next+4, v, mem_[m2w(next+4)]);
      reg_[dest2] = v;
      reg_[src] = next;
      DISPATCH();
  });

  REG_HANDLER(LDP_IP, {
      const register uint32_t dest1 = reg1(word);
      const register uint32_t dest2 = reg2(word);
      const register uint32_t src = reg3(word);
      const register uint32_t cur = regv(src, pc, reg_);
      const register uint32_t next = cur + ext11bit(word);
      register int32_t v;
      TIMER_READ(cur, v, mem_[m2w(cur)]);
      reg_[dest1] = v;
      TIMER_READ(cur+4, v, mem_[m2w(cur+4)]);
      reg_[dest2] = v;
      reg_[src] = next;
      DISPATCH();
  });

  REG_HANDLER(STOR_RI, {
      const register uint32_t addr = (word >> 11) & 0x1FFFFF;
      const register auto v = mem_[m2w(addr)] = regv(reg1(word), pc, reg_);
      VSIG(addr);
      TIMER_WRITE(addr, v);
      DISPATCH();
  });

  REG_HANDLER(STOR_IX, {
      const register uint32_t addr = regv(reg1(word), pc, reg_) + ext16bit(word);
      const register auto v = mem_[m2w(addr)] = regv(reg2(word), pc, reg_);
      VSIG(addr);
      TIMER_WRITE(addr, v);
      DISPATCH();
  });

  REG_HANDLER(STOR_PI, {
      const register uint32_t idx = reg1(word);
      const register uint32_t next = regv(idx, pc, reg_) + ext16bit(word);
      const register auto v = mem_[m2w(next)] = regv(reg2(word), pc, reg_);
      reg_[idx] = next;
      VSIG(next);
      TIMER_WRITE(next, v);
      DISPATCH();
  });

  REG_HANDLER(STOR_IP, {
      const register uint32_t idx = reg1(word);
      const register uint32_t cur = regv(idx, pc, reg_);
      const register uint32_t next = cur + ext16bit(word);
      const register auto v = mem_[m2w(cur)] = regv(reg2(word), pc, reg_);
      reg_[idx] = next;
      VSIG(cur);
      TIMER_WRITE(cur, v);
      DISPATCH();
  });

  REG_HANDLER(STP_PI, {
      const register uint32_t dest = reg1(word);
      const register uint32_t next = regv(dest, pc, reg_) + ext11bit(word);
      register auto v = mem_[m2w(next)] = regv(reg2(word), pc, reg_);
      VSIG(next);
      TIMER_WRITE(next, v);
      v = mem_[m2w(next+4)] = regv(reg3(word), pc, reg_);
      VSIG(next+4);
      TIMER_WRITE(next+4, v);
      reg_[dest] = next;
      DISPATCH();
  });

  REG_HANDLER(STP_IP, {
      const register uint32_t dest = reg1(word);
      const register uint32_t cur = regv(dest, pc, reg_);
      const register uint32_t next = cur + ext11bit(word);
      register auto v = mem_[m2w(cur)] = regv(reg2(word), pc, reg_);
      TIMER_WRITE(cur, v);
      VSIG(cur);
      v = mem_[m2w(cur+4)] = regv(reg3(word), pc, reg_);
      TIMER_WRITE(cur+4, v);
      VSIG(cur+4);
      reg_[dest] = next;
      DISPATCH();
  });

  REG_HANDLER(ADD_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc ,reg_) + regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(ADD_RI, {
      const register uint32_t idx = reg1(word);
      reg_[idx] = static_cast<int32_t>(regv(reg2(word), pc, reg_) + ext16bit(word));
      DISPATCH();
  });

  REG_HANDLER(SUB_RR, {
      const register uint32_t idx = reg1(word);
      const register uint32_t op2 = (~regv(reg3(word), pc, reg_) + 1);
      const register int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(SUB_RI, {
      const register uint32_t op2 = (~ext16bit(word) + 1);
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) + op2;
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(JMP, {
      pc = pc + reladdr26(word >> 6) - 4;
      DISPATCH();
  });

  REG_HANDLER(JNE, {
      if (reg_[reg1(word)] != 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(JEQ, {
      if (reg_[reg1(word)] == 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(JGT, {
      if (static_cast<int32_t>(reg_[reg1(word)]) > 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(JGE, {
      if (static_cast<int32_t>(reg_[reg1(word)]) >= 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(JLT, {
      if (static_cast<int32_t>(reg_[reg1(word)]) < 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(JLE, {
      if (static_cast<int32_t>(reg_[reg1(word)]) <= 0) pc = pc + reladdr21(word) - 4;
      DISPATCH();
  });

  REG_HANDLER(CALLI, {
      sp_ -= 4;
      mem_[m2w(sp_)] = pc;
      sp_ -= 4;
      mem_[m2w(sp_)] = fp_;
      fp_ = sp_;
      pc = pc + reladdr26(word >> 6) - 4;
      DISPATCH();
  });

  REG_HANDLER(CALLR, {
      sp_ -= 4;
      mem_[m2w(sp_)] = pc;
      sp_ -= 4;
      mem_[m2w(sp_)] = fp_;
      fp_ = sp_;
      pc = reg_[reg1(word)] - 4;
      DISPATCH();
  });

  REG_HANDLER(RET, {
      sp_ = fp_;
      fp_ = mem_[m2w(sp_)];
      sp_ += 4;
      pc = mem_[m2w(sp_)];
      sp_ += 4;
      mask_interrupt_ = false;
      DISPATCH();
  });

  REG_HANDLER(AND_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) & regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(AND_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) & v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(ORR_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) | regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(ORR_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) | v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(XOR_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) ^ regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(XOR_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) ^ v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LSL_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) << regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LSL_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) << v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LSR_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          (regv(reg2(word), pc, reg_) >> regv(reg3(word), pc, reg_));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(LSR_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) >> v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(ASR_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          static_cast<int32_t>((regv(reg2(word), pc, reg_)) >>
              regv(reg3(word), pc, reg_));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(ASR_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v =
          (static_cast<int32_t>(regv(reg2(word), pc, reg_)) >> v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(MUL_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) * regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(MUL_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) * v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(DIV_RR, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = regv(reg2(word), pc, reg_) / regv(reg3(word), pc, reg_);
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(DIV_RI, {
      const register uint32_t idx = reg1(word);
      const register int32_t v = (regv(reg2(word), pc, reg_) / v16bit(word));
      reg_[idx] = v;
      DISPATCH();
  });

  REG_HANDLER(MULL_RR, {
      const register uint32_t idxH = reg1(word);
      const register uint32_t idxL = reg2(word);
      const register int64_t v = regv(reg3(word), pc, reg_) * regv(reg4(word), pc, reg_);
      reg_[idxL] = (v & 0xFFFFFFFF);
      const register int32_t vH = (v >> 32);
      reg_[idxH] = vH;
      DISPATCH();
  });

  REG_HANDLER(WFI, {
    // Wait on mutex.
    std::unique_lock<std::mutex> ul(interrupt_mutex_);
    interrupt_event_.wait(ul, [this]{return interrupt_ > 0;});
    DISPATCH();
    return;
  });
}


const std::string CPU::PrintRegisters(bool hex) {
  std::stringstream ss;

  ss << "[";
  for (uint32_t i = 0; i < kRegCount; ++i) {
    if (i != 0) ss << " ";
    const uint32_t r = reg_[i];
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
  const auto opcode = word & 0x3F;
  auto pc = pc_;
  if (opcode == ISA::RET) {
      pc += 4;
  }
  ss << "0x" << std::hex << pc << ": " << std::dec;

  switch (opcode) {  // first 8 bits define the instruction.
    case ISA::HALT:
      ss << "halt";
      break;
    case ISA::NOP:
      ss << "nop";
      break;
    case ISA::LOAD_RI:
      ss << "load r" << reg1(word) << ", [0x" << std::hex << ((word >> 11) & 0x1FFFFF) << "]";
      break;
    case ISA::LOAD_IX:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << ", 0x" << std::hex << v16bit(word) << "]";
      break;
    case ISA::LOAD_IXR:
      ss << "load r" << reg1(word) << ", [r" << reg2(word) << ", r" << reg3(word) << "]";
      break;
    case ISA::LOAD_IP:
      ss << "load post inc r" << reg1(word) << ", [r" << reg2(word) << ", 0x" << std::hex << v16bit(word) << "]";
      break;
    case ISA::LOAD_PI:
      ss << "load pre inc r" << reg1(word) << ", [r" << reg2(word) << ", 0x" << std::hex << v16bit(word) << "]";
      break;
    case ISA::LDP_PI:
      ss << "load pre inc r" << reg1(word) << ", r" << reg2(word) << ", [r" << reg3(word) << ", 0x" << std::hex << ext11bit(word) << "]";
      break;
    case ISA::LDP_IP:
      ss << "load post inc r" << reg1(word) << ", r" << reg2(word) << ", [r" << reg3(word) << ", 0x" << std::hex << ext11bit(word) << "]";
      break;
    case ISA::STOR_RI:
      ss << "stor [0x" << std::hex << ((word >> 11) & 0x1FFFFF) << "], r" << std::dec << reg1(word);
      break;
    case ISA::STOR_IX:
      ss << "stor [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word);
      break;
    case ISA::STOR_IP:
      ss << "stor post inc [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word);
      break;
     case ISA::STOR_PI:
      ss << "stor pre inc [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word);
     case ISA::STP_PI:
      ss << "stor pre inc [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word) << ", r" << reg3(word);
     case ISA::STP_IP:
      ss << "stor post inc [r" << reg1(word) << ", 0x" << std::hex << v16bit(word) << "], r" << std::dec << reg2(word) << ", r" << reg3(word);
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
      ss << "jmp 0x" << std::hex << pc;
      break;
    case ISA::JNE:
      ss << "jne 0x" << std::hex << pc;
      break;
    case ISA::JEQ:
      ss << "jeq 0x" << std::hex << pc;
      break;
    case ISA::JGT:
      ss << "jgt 0x" << std::hex << pc;
      break;
    case ISA::JGE:
      ss << "jge 0x" << std::hex << pc;
      break;
    case ISA::JLT:
      ss << "jlt 0x" << std::hex << pc;
      break;
    case ISA::JLE:
      ss << "jle 0x" << std::hex << pc;
      break;
    case ISA::CALLI:
      ss << "call 0x" << std::hex << pc;
      break;
    case ISA::CALLR:
      ss << "call [r" << reg1(word) << "]";
      break;
    case ISA::RET:
      ss << "ret";
      break;
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
    case ISA::MUL_RI:
      ss << "mul r" << reg1(word) << ", r" << reg2(word) << ", 0x" << std::hex << v16bit(word);
      break;
    case ISA::MUL_RR:
      ss << "mul r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::MULL_RR:
      ss << "mull r" << reg1(word) << ", r" << reg2(word) << ", r" << reg3(word);
      break;
    case ISA::WFI:
      ss << "wfi";
      break;
    default:
      ss << "Unrecognizd instrucation: 0x" << std::hex << word;
      assert(false);
      break;
  }
  return ss.str();
}

}  // namespace gvm
