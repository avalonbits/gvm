#include "computer.h"

#include <iostream>
#include <thread>

namespace gvm {

void Computer::LoadRom(uint32_t memaddr, const Rom* rom) {
  assert(memaddr % kWordSize == 0);
  cpu_->LoadProgram(memaddr, rom->Contents());
}

void Computer::Run(const bool debug) {
  cpu_->SetPC(16 << 20);
  std::thread cpu_thread([this, debug]() {
    cpu_->Run(debug);
  });
  std::thread video_thread([this]() {
    video_controller_->Run();
  });

  cpu_thread.join();
  video_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  std::cerr << cpu_->PrintMemory(0x15FD00, 0x15FD00 + 256);
  std::cerr << cpu_->PrintStatusFlags();
}

}  // namespace gvm
