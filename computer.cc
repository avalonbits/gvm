#include "computer.h"

#include <iostream>
#include <thread>

namespace gvm {

void Computer::LoadRom(uint32_t memaddr, Rom* rom) {
  assert(memaddr % kWordSize == 0);
  roms_.emplace_back(std::make_pair(memaddr, rom));
}

void Computer::Run() {
  for (const auto& rom : roms_) {
    cpu_->LoadProgram(rom.first, rom.second->Contents());
  }
  cpu_->SetPC(16 << 20);
  std::thread cpu_thread([this]() {
    cpu_->Run();
  });
  std::thread video_thread([this]() {
    video_controller_->Run();
  });

  cpu_thread.join();
  video_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  std::cerr << cpu_->PrintMemory(0x400, 0x904);
  std::cerr << cpu_->PrintStatusFlags();
}

}  // namespace gvm
