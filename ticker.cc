#include "ticker.h"

#include <cassert>
#include <chrono>
#include <thread>

namespace gvm {

Ticker::Ticker(uint32_t hertz, std::function<void(void)> callback)
    : hertz_(hertz), callback_(callback), stop_(false) {
  assert(hertz >= 100);
}

void Ticker::Start() {
  auto start = std::chrono::high_resolution_clock::now();
  const uint64_t nsecs = 1000000000;
  const std::chrono::nanoseconds exp_sleep(nsecs/hertz_);
  std::chrono::nanoseconds sleep = exp_sleep; 

  register uint64_t ticks = 1;
  while (!stop_) {
    std::this_thread::sleep_for(sleep);
    callback_();
    ++ticks;
    const auto total = (std::chrono::high_resolution_clock::now() - start);
    sleep = exp_sleep*ticks - total;
  } 
}

}  // namespace gvm
