#ifndef _GVM_TIMER_H_
#define _GVM_TIMER_H_

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
    SetupOneShot(msec, one_shot);
  }

 private:
  void SetupOneShot(uint32_t msec, std::function<void(uint32_t)> one_shot) const {
    std::thread one_shot_thread([this, msec, one_shot]() {
      std::chrono::milliseconds duration(msec);
      std::this_thread::sleep_for(duration);
      one_shot(Elapsed().count() / 1000000);
    });
    one_shot_thread.detach();
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class TimerService {
 public:
  explicit TimerService(SyncChan<uint32_t>* chan) : chan_(chan) {}

  void SetOneShot(std::function<void(uint32_t)> one_shot) {
    one_shot_ = one_shot;
    has_one_shot_ = true;
  }

  void SetRecurring(std::function<void(void)> recurring) {
    recurring_ = recurring;
    has_recurring_ = true;
  } 

  void OneShot(uint32_t msec);
  void Start();
  void Reset();
  void Stop();
  uint32_t Elapsed();

 private:
  Timer timer_;
  SyncChan<uint32_t>* chan_;
  std::function<void(uint32_t)> one_shot_;
  bool has_one_shot_;
  std::function<void(void)> recurring_;
  bool has_recurring_;
};

}  // namespace gvm

#endif  // _GVM_TIMER_H_
