#include "computer.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace {

const uint32_t kKernelMemSize = 1024 * 1024;
const uint32_t kVramSize = 640 * 360 * 4;
const uint32_t kUnmappedVram = 1024 * 1024 - kVramSize;
const uint32_t kUserMemSize = 15 * 1024 * 1024 + kUnmappedVram;
const uint32_t kUnicodeBitmapFont = 1024 * 1024;
const uint32_t kIOMemSize = 8;
const uint32_t kMemLimit =
    kKernelMemSize + kUserMemSize + kVramSize + kUnicodeBitmapFont + kIOMemSize;
const uint32_t kVramStart = kKernelMemSize + kUserMemSize;
const uint32_t kUnicodeRomStart = kVramStart + kVramSize;
const uint32_t kIOStart = kUnicodeRomStart + kUnicodeBitmapFont;
const uint32_t kVramReg = kIOStart;
const uint32_t kInputReg = kIOStart + 4;
const uint32_t kTimerReg = kInputReg + 4;
const uint32_t kOneShotReg = kTimerReg + 4;
const uint32_t kRecurringReg = kOneShotReg + 4;
const int kFrameBufferW = 640;
const int kFrameBufferH = 360;

}  // namespace

namespace gvm {

Computer::Computer(CPU* cpu, VideoController* video_controller)
    : mem_size_bytes_(kMemLimit),
      mem_(new uint32_t[mem_size_bytes_/kWordSize]),
      cpu_(cpu), video_controller_(video_controller) {
  assert(mem_ != nullptr);
  assert(cpu_ != nullptr);
  assert(video_controller_ != nullptr);
  memset(mem_.get(), 0, mem_size_bytes_);
  video_controller_->SetInputController(new InputController(
      [this](uint32_t value) {
    mem_.get()[kInputReg/kWordSize] = value;
    cpu_->Input();
    std::this_thread::yield();
  }));
  video_controller_->SetSignal(&video_signal_);
  cpu_->ConnectMemory(mem_.get(), mem_size_bytes_, kVramStart);
  cpu_->SetVideoSignal(kVramReg, &video_signal_);

  timer_service_.reset(new TimerService(&timer_chan_));
  timer_service_->SetOneShot([this](uint32_t elapsed) {
    mem_.get()[kOneShotReg / kWordSize] = elapsed;
    cpu_->Timer();
    std::this_thread::yield();
  });
  timer_service_->SetRecurring([this](uint32_t elapsed) {
    mem_.get()[kRecurringReg / kWordSize] = elapsed;
    cpu_->RecurringTimer();
    std::this_thread::yield();
  });
  cpu_->SetTimerSignal(kTimerReg, kOneShotReg, kRecurringReg, timer_service_.get());
  RegisterVideoDMA();
}


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

  auto* timer = timer_service_.get();
  std::thread timer_thread([timer]() {
    timer->Start();
  });

  uint32_t elapsed;
  std::thread cpu_thread([this, timer, &elapsed, &runtime, &op_count]() {
    timer->Reset();
    const auto start = std::chrono::high_resolution_clock::now();
    op_count = cpu_->PowerOn();
    runtime = std::chrono::high_resolution_clock::now() - start;
    elapsed = timer->Elapsed();
    timer->Stop();
    video_controller_->Shutdown();
  });

  // This has to run on the main thread or it won't render using OpenGL ES.
  video_controller_->Run();
  timer_thread.join();
  cpu_thread.join();

  std::cerr << cpu_->PrintRegisters(/*hex=*/true);
  const auto time = runtime.count();
  const auto per_inst = time / static_cast<double>(op_count);
  auto average_clock = 1000000000 / per_inst / 1000000;
  std::string hz = "MHz";
  if (average_clock < 1.0) {
    average_clock *= 1000.0;
    hz = "KHz";
  }
  std::cerr << "CPU Runtime: " << (time / static_cast<double>(1000)) << "us\n";
  std::cerr << "CPU Instruction count: " << op_count << std::endl;
  std::cerr << "Average per instruction: " << per_inst << "ns\n";
  std::cerr << "Average clock: " << average_clock << hz << "\n";
  std::cerr << "Timer elapsed: " << (elapsed /10.0) << "ms\n";
}

void Computer::RegisterVideoDMA() {
  assert(video_controller_ != nullptr);
  video_controller_->RegisterDMA(
      kVramReg, kVramStart/kWordSize, kFrameBufferW, kFrameBufferH,
      32, mem_.get());
}

}  // namespace gvm
