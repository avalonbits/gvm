#ifndef _GVM_COMPUTER_H_
#define _GVM_COMPUTER_H_

#include <cassert>
#include <memory>
#include <utility>

#include "cpu.h"
#include "rom.h"
#include "ticker.h"
#include "video_controller.h"

namespace gvm {

class Computer {
 public:
  // Owns cpu and video_display.
  Computer(uint32_t mem_size_bytes, CPU* cpu, VideoController* video_controller,
           const bool shutdown_on_halt)
      : mem_size_bytes_(mem_size_bytes),
        mem_(new uint32_t[mem_size_bytes/kWordSize]),
        cpu_(cpu), video_controller_(video_controller),
        shutdown_on_halt_(shutdown_on_halt) {
    assert(mem_ != nullptr);
    assert(cpu_ != nullptr);
    assert(video_controller_ != nullptr);
    memset(mem_.get(), 0, mem_size_bytes);
    ticker_.reset(new Ticker(1000, [this]() {
      cpu_->Tick();
    }));
    cpu_->ConnectMemory(mem_.get(), mem_size_bytes);
    RegisterVideoDMA();
  }

  // Takes ownership of rom.
  void LoadRom(const Rom* rom);

  void Run();
  void Shutdown();

 private:
  void RegisterVideoDMA();

  const uint32_t mem_size_bytes_;
  std::unique_ptr<uint32_t> mem_;
  std::unique_ptr<CPU> cpu_;
  std::unique_ptr<VideoController> video_controller_;
  std::unique_ptr<Ticker> ticker_;
  const bool shutdown_on_halt_;
};

}  // namespace gvm

#endif  // _GVM_COMPUTER_H_
