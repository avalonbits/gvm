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

#ifndef _GVM_TIMER_H_
#define _GVM_TIMER_H_

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <thread>

#include "sync_types.h"

namespace gvm {

class Timer {
 public:
  Timer() : start_(std::chrono::high_resolution_clock::now()) {}
  ~Timer() {}

  void Reset() {
    start_ = std::chrono::high_resolution_clock::now();
  }

  std::chrono::nanoseconds Elapsed() const {
    return std::chrono::high_resolution_clock::now() - start_;
  }

  void OneShot(uint32_t msec, std::function<void(uint32_t)> one_shot) const {
    std::thread one_shot_thread([this, msec, one_shot]() {
      std::chrono::milliseconds duration(msec);
      std::this_thread::sleep_for(duration);
      one_shot(Elapsed().count() / 1000000);
    });
    one_shot_thread.detach();
  }

  void Recurring(uint32_t hz, std::atomic_bool* done, std::atomic_bool* ack_done,
                 std::function<void(uint32_t)> recurring);

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class TimerService {
 public:
  explicit TimerService(SyncChan<uint32_t>* chan) : chan_(chan) {
    assert(chan_ != nullptr);
  }

  void SetOneShot(std::function<void(uint32_t)> one_shot) {
    one_shot_ = one_shot;
    has_one_shot_ = true;
  }

  void SetRecurring(std::function<void(uint32_t)> recurring) {
    recurring_ = recurring;
    has_recurring_ = true;
  }

  void OneShot(uint32_t msec);
  void Recurring(uint32_t hertz);
  void CancelRecurring();

  void Start();
  void Reset();
  void Stop();
  uint32_t Elapsed();

 private:
  Timer timer_;
  SyncChan<uint32_t>* chan_;
  std::function<void(uint32_t)> one_shot_;
  bool has_one_shot_;
  std::function<void(uint32_t)> recurring_;
  bool has_recurring_;
};

}  // namespace gvm

#endif  // _GVM_TIMER_H_
