#ifndef _GVM_CPU_H_
#define _GVM_CPU_H_

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <cstdint>
#include <string>
#include <vector>

#include "isa.h"
#include "sync_types.h"
#include "timer.h"
#include "video_controller.h"

namespace gvm {
class CPU {
 public:
  CPU();

  // Don't allow copy construction
  CPU(const CPU&) = delete;
  CPU& operator=(const CPU&) = delete;

  void ConnectMemory(
      uint32_t* mem, uint32_t mem_size_bytes, uint32_t user_ram_limit);

  uint32_t PowerOn();
  uint32_t Reset();

  // Sets signal for input handling.
  void Input();

  void SetVideoSignal(const uint32_t vram_reg, SyncPoint* video_signal) {
    vram_reg_ = vram_reg;
    video_signal_ = video_signal;
  }

  void SetTimerSignal(const uint32_t timer_reg, TimerService* timer_signal) {
    timer_reg_ = timer_reg;
    timer_signal_ = timer_signal;
  }

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  void Run();
  void SetPC(uint32_t pc);

  std::string PrintInstruction(const Word word);

  uint32_t& pc_;
  uint32_t* mem_;
  uint32_t reg_[kRegCount];
  uint32_t mem_size_;
  uint32_t user_ram_limit_;
  uint32_t& sp_;
  uint32_t& fp_;
  uint32_t op_count_;
  volatile bool mask_interrupt_;
  volatile uint32_t interrupt_;
  std::mutex interrupt_mutex_;
  std::condition_variable interrupt_event_;
  uint32_t vram_reg_;
  SyncPoint* video_signal_;
  uint32_t timer_reg_;
  TimerService* timer_signal_;
};

}  // namespace gvm

#endif  // _GVM_CPU_H_
