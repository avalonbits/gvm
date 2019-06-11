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
const uint32_t kIOMemSize = 1024;
const uint32_t kColorTableSize = 1024;
const uint32_t kMemLimit =
    kKernelMemSize + kUserMemSize + kVramSize + kUnicodeBitmapFont +
    kColorTableSize + kIOMemSize;
const uint32_t kVramStart = kKernelMemSize + kUserMemSize;
const uint32_t kUnicodeRomStart = kVramStart + kVramSize;
const uint32_t kColorTableStart = kUnicodeRomStart + kUnicodeBitmapFont;
const uint32_t kIOStart = kColorTableStart + kColorTableSize;
const uint32_t kVramReg = kIOStart;
const uint32_t kInputReg = kIOStart + 4;
const uint32_t kTimerReg = kInputReg + 4;
const uint32_t kOneShotReg = kTimerReg + 4;
const uint32_t kRecurringReg = kOneShotReg + 4;
const uint32_t kOneShot2Reg = kRecurringReg + 4;
const uint32_t kRecurring2Reg = kOneShot2Reg + 4;
const uint32_t kDisksReg = kRecurring2Reg + 4;
const int kFrameBufferW = 640;
const int kFrameBufferH = 360;

}  // namespace

namespace gvm {

Computer::Computer(CPU* cpu, VideoController* video_controller,
                   DiskController* disk_controller)
    : mem_size_bytes_(kMemLimit),
      mem_(new uint32_t[mem_size_bytes_/kWordSize]),
      cpu_(cpu), video_controller_(video_controller), disk_controller_(disk_controller) {
  assert(mem_ != nullptr);
  assert(cpu_ != nullptr);
  assert(video_controller_ != nullptr);
  memset(mem_.get(), 0x0, mem_size_bytes_);
  video_controller_->SetInputController(new InputController(
      [this](uint32_t value) {
    mem_.get()[kInputReg/kWordSize] = value;
    cpu_->Input();
    std::this_thread::yield();
  }));
  video_controller_->SetSignal(&video_signal_);
  video_controller_->SetTextRom(&mem_.get()[kUnicodeRomStart/kWordSize]);
  video_controller_->SetColorTable(&mem_.get()[kColorTableStart/kWordSize]);
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

  timer2_service_.reset(new TimerService(&timer2_chan_));
  timer2_service_->SetOneShot([this](uint32_t elapsed) {
    mem_.get()[kOneShot2Reg / kWordSize] = elapsed;
    cpu_->Timer2();
    std::this_thread::yield();
  });
  timer2_service_->SetRecurring([this](uint32_t elapsed) {
    mem_.get()[kRecurring2Reg / kWordSize] = elapsed;
    cpu_->RecurringTimer2();
    std::this_thread::yield();
  });

  cpu_->SetTimerSignal(kTimerReg, kOneShotReg, kRecurringReg, timer_service_.get());
  cpu_->SetTimer2Signal(kOneShot2Reg, kRecurring2Reg, timer2_service_.get());

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

  auto* timer2 = timer2_service_.get();
  std::thread timer2_thread([timer2]() {
    timer2->Start();
  });


  uint32_t elapsed;
  std::thread cpu_thread([this, timer, timer2, &elapsed, &runtime, &op_count]() {
    timer->Reset();
    timer2->Reset();
    const auto start = std::chrono::high_resolution_clock::now();
    op_count = cpu_->PowerOn();
    runtime = std::chrono::high_resolution_clock::now() - start;
    elapsed = timer->Elapsed();
    timer->Stop();
    timer2->Stop();
    video_controller_->Shutdown();
  });

  // This has to run on the main thread or it won't render using OpenGL ES.
  video_controller_->Run();
  timer_thread.join();
  timer2_thread.join();
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
