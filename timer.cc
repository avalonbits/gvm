#include "timer.h"

namespace gvm {

void TimerService::Start() {
  while (true) {
    auto cmd = chan_->recv();
    if (cmd <= 0) break;

    if (cmd == 1) {
      std::chrono::nanoseconds elapsed = timer_.Elapsed();
      const uint32_t usec = elapsed.count() / 1000;
      chan_->send(usec);
      continue;
    }

    if (cmd == 2) {
      timer_.Reset();
      continue;
    }

    std::cerr << "Unkown command " << cmd << std::endl;
  }
}

void TimerService::Stop() {
  chan_->send(0);
}

uint32_t TimerService::Elapsed() {
  chan_->send(1);
  return chan_->recv();
}

void TimerService::Reset() {
  chan_->send(2);
}

}  // namespace gvm
