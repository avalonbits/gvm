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

#ifndef _GVM_SYNC_TYPES_H_
#define _GVM_SYNC_TYPES_H_

#include <atomic>
#include <functional>
#include <mutex>
#include <iostream>
#include <thread>

namespace gvm {

template <typename VALUE>
class SyncChan {
 public:
  SyncChan() : closed_(false) {
    closed_.store(false);
    sync_.lock();
  }

  void send(VALUE value) {
    if (closed_.load()) {
      sync_.unlock();
      return;
    }
    {
      std::unique_lock<std::mutex> lk(writer_);
      while (reader_.try_lock()) {
        reader_.unlock();
        if (closed_.load()) {
          sync_.unlock();
          return;
        }
        std::this_thread::yield();
      }

      value_ = value;
      sync_.unlock();
      std::this_thread::yield();
    }
  }

  VALUE recv() {
    if (closed_.load()) return value_;
    VALUE value;
    {
      std::unique_lock<std::mutex> lk1(reader_);
      sync_.lock();
      value = value_;
    }
    return value;
  }

  void Close() {
    closed_.store(true);
    sync_.unlock();
  }

 private:
  std::mutex reader_;
  std::mutex writer_;
  std::mutex sync_;
  std::atomic_bool closed_;
  VALUE value_;
};

class SyncPoint {
 public:
  void send() {
    chan_.send(true);
  }

  void recv() {
    chan_.recv();
  }

  void Close() { chan_.Close(); }

 private:
  SyncChan<bool> chan_;
};


}  // namespace

#endif  // _GVM_SYNC_TYPES_H_
