#include "computer.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace gvm {

void Computer::LoadRom(uint32_t memaddr, const Rom* rom) {
  assert(memaddr % kWordSize == 0);
  cpu_->LoadProgram(memaddr, rom->Contents());
}

void Computer::Run(const bool debug) {
  cpu_->SetPC(16 << 20);
  std::chrono::nanoseconds runtime;
  int op_count;
  std::thread cpu_thread([this, debug, &runtime, &op_count]() {
    const auto start = std::chrono::high_resolution_clock::now();
    op_count = cpu_->Run(debug);
    runtime = std::chrono::high_resolution_clock::now() - start;
  });
  std::thread video_thread([this]() {
    video_controller_->Run();
  });

  cpu_thread.join();
  video_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  std::cerr << cpu_->PrintStatusFlags();
  const auto time = runtime.count();
  const auto per_inst = time / static_cast<double>(op_count);
  std::cerr << "Runtime: " << (time / static_cast<double>(1000)) << "us\n";
  std::cerr << "Average per instruction: " << per_inst << "ns\n";
}

}  // namespace gvm
