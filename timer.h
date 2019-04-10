#ifndef _GVM_TIMER_H_
#define _GVM_TIMER_H_

#include <chrono>
#include <cstdint>

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

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class TimerService {
 public:
  explicit TimerService(SyncChan<uint32_t>* chan) : chan_(chan) {}

  void Start();
  void Reset();
  void Stop();
  uint32_t Elapsed();

 private:
  Timer timer_;
  SyncChan<uint32_t>* chan_;
};

}  // namespace gvm

#endif  // _GVM_TIMER_H_
