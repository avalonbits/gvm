#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "sync_chan.h"
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
