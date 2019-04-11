#include "timer.h"

namespace gvm {

void Timer::Recurring(
    uint32_t hz, std::atomic_bool* done, std::atomic_bool* ack_done,
    std::function<void(uint32_t)> recurring) {
  std::thread recurring_thread([this, hz, done, ack_done, recurring]() {
    auto start = std::chrono::high_resolution_clock::now();
    const uint64_t nsecs = 1000000000;
    const std::chrono::nanoseconds exp_sleep(nsecs/hz);
    std::chrono::nanoseconds sleep = exp_sleep;

    register uint64_t ticks = 0;
    while (!done->load()) {
      std::this_thread::sleep_for(sleep);
      recurring(Elapsed().count() / 1000000);
      ++ticks;
      const auto total = (std::chrono::high_resolution_clock::now() - start);
      sleep = exp_sleep*(ticks+1) - total;
    }
    ack_done->store(true);
  });
  recurring_thread.detach();
}


void TimerService::Start() {
  std::atomic_bool done_recurring(false);
  std::atomic_bool ack_done(true);
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
      timer_.OneShot(value >> 4, one_shot_);
      continue;
    }

    if ((cmd == 4 || cmd == 5) && has_recurring_) {
      // Cancel any existing thread.
      const bool done = done_recurring.exchange(true);
      if (!done) {
        while (!ack_done.load()) {}
      }
      done_recurring.store(false);

      if (cmd == 4) {
        ack_done.store(false);
        timer_.Recurring(value >> 4, &done_recurring, &ack_done, recurring_);
      }
      continue;
    }
    std::cerr << "Unkown command " << cmd << std::endl;
  }
  const bool done = done_recurring.exchange(true);
  if (!done) {
    while (!ack_done.load()) {}
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

void TimerService::Recurring(uint32_t hertz) {
  chan_->send(4 | hertz << 4);
}

void TimerService::CancelRecurring() {
  chan_->send(5);
}

}  // namespace gvm
