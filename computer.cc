#include "computer.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace gvm {

void Computer::LoadRom(const Rom* rom) {
  cpu_->LoadProgram(rom->Contents());
  delete rom;
}

void Computer::Run(const bool debug) {
  cpu_->SetPC(16 << 20);
  std::chrono::nanoseconds runtime;
  int op_count;

  std::thread ticker_thread([this]() {
      ticker_->Start();
  });

  std::thread cpu_thread([this, debug, &runtime, &op_count]() {
    const auto start = std::chrono::high_resolution_clock::now();
    op_count = cpu_->Run(debug);
    runtime = std::chrono::high_resolution_clock::now() - start;
  });
  std::thread video_thread([this]() {
    video_controller_->Run();
  });

  cpu_thread.join();
  ticker_->Stop();
  if (shutdown_on_halt_) video_controller_->Shutdown();
  video_thread.join();
  ticker_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  const auto time = runtime.count();
  const auto per_inst = time / static_cast<double>(op_count);
  const auto average_clock = 1000000000 / per_inst / 1000000;
  std::cerr << "CPU Runtime: " << (time / static_cast<double>(1000)) << "us\n";
  std::cerr << "CPU Instruction count: " << op_count << std::endl;
  std::cerr << "Average per instruction: " << per_inst << "ns\n";
  std::cerr << "Average clock: " << average_clock << "MHz\n";
}

}  // namespace gvm
