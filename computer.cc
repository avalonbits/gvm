#include "computer.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace {

static const uint32_t kVideoMemReg = 0x80;
static const uint32_t kCpuJiffiesReg = 0x04;
static const uint32_t kVideoMemStart = 0x84;
static const uint32_t kVideoMemSizeWords = 640 * 360;
static const uint32_t kVideoMemEnd = kVideoMemStart + kVideoMemSizeWords;
static const int kFrameBufferW = 640;
static const int kFrameBufferH = 360;

}  // namespace

namespace gvm {

void Computer::LoadRom(const Rom* rom) {
  const auto& program = rom->Contents();
  for (const auto& kv : program) {
    const auto& start = kv.first / kWordSize;
    const auto& words = kv.second;
    assert(!words.empty());
    assert(words.size() + start < mem_size_bytes_ / kWordSize);
    for (uint32_t idx = start, i = 0; i < words.size(); ++idx, ++i) {
      mem_.get()[idx] = words[i];
    }
  }
}

void Computer::Run() {
  std::chrono::nanoseconds runtime;
  uint32_t op_count;

  std::thread ticker_thread([this]() {
      ticker_->Start();
  });

  std::thread cpu_thread([this, &runtime, &op_count]() {
    const auto start = std::chrono::high_resolution_clock::now();
    op_count = cpu_->PowerOn();
    runtime = std::chrono::high_resolution_clock::now() - start;
    ticker_->Stop();
    if (shutdown_on_halt_) video_controller_->Shutdown();
  });

  // This has to run on the main thread or it won't render using OpenGL ES.
  video_controller_->Run();

  cpu_thread.join();
  ticker_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  std::cerr << cpu_->PrintMemory(0xE1084, 0xE1088);
  const auto time = runtime.count();
  const auto per_inst = time / static_cast<double>(op_count);
  const auto average_clock = 1000000000 / per_inst / 1000000;
  std::cerr << "CPU Runtime: " << (time / static_cast<double>(1000)) << "us\n";
  std::cerr << "CPU Instruction count: " << op_count << std::endl;
  std::cerr << "Average per instruction: " << per_inst << "ns\n";
  std::cerr << "Average clock: " << average_clock << "MHz\n";
}

void Computer::RegisterVideoDMA() {
  assert(video_controller_ != nullptr);
  video_controller_->RegisterDMA(
      kVideoMemReg, kVideoMemStart / kWordSize, kFrameBufferW, kFrameBufferH,
      32, mem_.get());
}

}  // namespace gvm
