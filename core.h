/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GVM_CORE_H_
#define _GVM_CORE_H_

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

template<typename MEMORY>
class Core {
 public:
  Core();

  // Don't allow copy construction
  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;

  void ConnectMemory(MEMORY, uint32_t user_ram_limit);

  uint64_t PowerOn();
  uint64_t Reset();

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
  void Run();
  void InterruptService(uint32_t& pc);
  void SetPC(uint32_t pc);

  std::string PrintInstruction(const Word word);

  uint32_t& pc_;
  MEMORY mem_;
  uint32_t reg_[kRegCount];
  uint32_t user_ram_limit_;
  uint32_t& sp_;
  uint32_t& fp_;
  uint64_t op_count_;
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

  typedef std::function<void(uint32_t, uint32_t&, bool&)> Handler;
  Handler handlers_[64];
};

}  // namespace gvm

#endif  // _GVM_CORE_H_
