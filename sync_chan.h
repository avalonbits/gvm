#ifndef _GVM_SYNC_CHAN_H_
#define _GVM_SYNC_CHAN_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <iostream>

namespace gvm {

template <typename VALUE>
class SyncChan {
 public:
  SyncChan() : full_(false), reader_(false), writer_(false) {}

  void send(VALUE value) {
    // Make sure we are the only writer.
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !writer_; });
      writer_ = true;
    }

    // Make sure there is a reader available to receive the data.
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return reader_; });
    }

    // Wait for the channel to be empty, then write to it.
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !full_; });
      value_ = value;
      full_ = true;  // Channel has data.
      writer_ = false;
    }
    cond_.notify_all();
  }

  VALUE recv() {
    // Make sure we are the only reader in the channel.
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !reader_; });
      reader_ = true;
    }
    cond_.notify_all();

    // Now wait for data to be sent to the channel and read it.
    VALUE value;
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return full_; });
      value = value_;
      full_ = false;  // channel has been drained.
      reader_ = false;
    }
    cond_.notify_all();

    return value;
  }

 private:
  bool full_;
  bool reader_;
  bool writer_;
  std::mutex mu_;
  std::condition_variable cond_;
  VALUE value_;
};

}  // namespace

#endif  // _GVM_SYNC_CHAN_H_
