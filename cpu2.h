#ifndef _GVM_CPU2_H_
#define _GVM_CPU2_H_

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
class CPU2 {
 public:
  CPU2();

  // Don't allow copy construction
  CPU2(const CPU2&) = delete;
  CPU2& operator=(const CPU2&) = delete;

  void ConnectMemory(
      uint32_t* mem, uint32_t mem_size_bytes, uint32_t user_ram_limit);

  uint32_t PowerOn();
  uint32_t Reset();

  // Sets signal for input handling.
  void Input();
  void Timer();
  void RecurringTimer();
  void Timer2();
  void RecurringTimer2();

  void SetVideoSignal(const uint32_t vram_reg, SyncPoint* video_signal) {
    vram_reg_ = vram_reg;
    video_signal_ = video_signal;
  }

  void SetTimerSignal(
      const uint32_t timer_reg, const uint32_t oneshot_reg,
      const uint32_t recurring_reg, TimerService* timer_signal) {
    timer_reg_ = timer_reg;
    oneshot_reg_ = oneshot_reg;
    recurring_reg_ = recurring_reg;
    timer_signal_ = timer_signal;
  }

  void SetTimer2Signal(
      const uint32_t oneshot_reg, const uint32_t recurring_reg,
      TimerService* timer_signal) {
    oneshot2_reg_ = oneshot_reg;
    recurring2_reg_ = recurring_reg;
    timer2_signal_ = timer_signal;
  }

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  void Nop(uint32_t Word) {}
  void Halt(uint32_t word) { halt_ = true; }
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
  uint32_t oneshot_reg_;
  uint32_t recurring_reg_;
  TimerService* timer_signal_;
  uint32_t oneshot2_reg_;
  uint32_t recurring2_reg_;
  TimerService* timer2_signal_;
  bool halt_;
  // Table of instruction handlers.
  std::function<void(uint32_t)> handlers_[64];
};

}  // namespace gvm

#endif  // _GVM_CPU2_H_
