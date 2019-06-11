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

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "sync_types.h"
using gvm::SyncChan;

void thread1chan(SyncChan<int>* chan) {
  bool stop = false;
  while (!stop) {
    int v = chan->recv();
    chan->send(++v);
    std::cerr << "Thread 1: " << v << std::endl;
    if (v >= 100) {
      stop = true;
    }
  }
  std::cerr << "Thread 1: bye!\n";
}

void thread2chan(SyncChan<int>* chan) {
  bool stop = false;
  int v = 0;
  while (!stop) {
    chan->send(v);
    std::cerr << "Thread 2: " << v << std::endl;
    v = chan->recv();
    if (v >= 100) stop = true;
    ++v;
  }

  std::cerr << "Thread 2: bye!\n";

}



int main(void) {
    SyncChan<int> chan;
    std::thread t1(&thread1chan, &chan);
    std::thread t2(&thread2chan, &chan);

    t1.join();
    t2.join();
    return 0;
}
