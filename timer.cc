#include "timer.h"

namespace gvm {

void TimerService::Start() {
  while (true) {
    auto value = chan_->recv();
    const auto cmd = value & 0xF;
    if (cmd <= 0) break;

    if (cmd == 1) {
      std::chrono::nanoseconds elapsed = timer_.Elapsed();
      const uint32_t tenth_msec = elapsed.count() / 100000;
      chan_->send(tenth_msec);
      continue;
    }

    if (cmd == 2) {
      timer_.Reset();
      continue;
    }

    if (cmd == 3 && has_one_shot_) {
      timer_.OneShot(value >> 3, one_shot_);
    }

    std::cerr << "Unkown command " << cmd << std::endl;
  }
}

void TimerService::Stop() {
  chan_->send(0);
  chan_->Close();
}

uint32_t TimerService::Elapsed() {
  chan_->send(1);
  return chan_->recv();
}

void TimerService::Reset() {
  chan_->send(2);
}

void TimerService::OneShot(uint32_t msec) {
  chan_->send(3 | msec << 4);
}

}  // namespace gvm
