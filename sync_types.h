#ifndef _GVM_SYNC_TYPES_H_
#define _GVM_SYNC_TYPES_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <iostream>

namespace gvm {

template <typename VALUE>
class SyncChan {
 public:
  SyncChan() : sync_(false), receiver_waiting_(false), closed_(false) {}

  void send(VALUE value) {
    if (closed_) return;

    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return closed_ || receiver_waiting_; });
      if (closed_) {
        cond_.notify_all();
        return;
      }
    }

    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return closed_ || !sync_; });
      if (closed_) {
        cond_.notify_all();
        return;
      }
      value_ = value;
      sync_ = true;
    }
    cond_.notify_all();
  }

  VALUE recv() {
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !receiver_waiting_; });
      receiver_waiting_ = true;
    }
    cond_.notify_all();

    VALUE value;
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return closed_ || sync_; });
      value = value_;
      sync_ = false;
      receiver_waiting_ = false;
    }
    cond_.notify_all();
    return value;
  }

  void Close() {
    {
      std::lock_guard<std::mutex> lk(mu_);
      closed_ = true;
    }
    cond_.notify_all();
  }

 private:
  volatile bool sync_;
  volatile bool receiver_waiting_;
  volatile bool closed_;
  std::mutex mu_;
  std::condition_variable cond_;
  VALUE value_;
};

class SyncPoint {
 public:
  SyncPoint() : sync_(false), receiver_waiting_(false) {}

  void send() {
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return receiver_waiting_; });
    }
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !sync_; });
      sync_ = true;
    }
    cond_.notify_all();
  }

  void recv() {
    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return !receiver_waiting_; });
      receiver_waiting_ = true;
    }
    cond_.notify_all();

    {
      std::unique_lock<std::mutex> lk(mu_);
      cond_.wait(lk, [this]() { return sync_; });
      sync_ = false;
      receiver_waiting_ = false;
    }
    cond_.notify_all();
  }

 private:
  bool sync_;
  bool receiver_waiting_;
  std::mutex mu_;
  std::condition_variable cond_;
};


}  // namespace

#endif  // _GVM_SYNC_TYPES_H_
