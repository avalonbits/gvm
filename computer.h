#ifndef _GVM_COMPUTER_H_
#define _GVM_COMPUTER_H_

#include <cassert>
#include <memory>
#include <utility>

#include "cpu.h"
#include "rom.h"
#include "video_display.h"

namespace gvm {

class Computer {
 public:
  // Owns cpu and video_display.
  Computer(uint32_t mem_size_bytes, CPU* cpu, VideoDisplay* video_display)
      : mem_size_bytes_(mem_size_bytes),
        mem_(new uint32_t[mem_size_bytes/kWordSize]),
        cpu_(cpu), video_display_(video_display) {
    assert(mem_ != nullptr);
    assert(cpu_ != nullptr);
    assert(video_display_ != nullptr);
    cpu_->ConnectMemory(mem_.get(), mem_size_bytes);
  }

  // Takes ownership of rom.
  void LoadRom(uint32_t memaddr, Rom* rom);

  void Run();
  void Shutdown();

 private:
  const uint32_t mem_size_bytes_;
  std::unique_ptr<uint32_t> mem_;
  std::unique_ptr<CPU> cpu_;
  std::unique_ptr<VideoDisplay> video_display_;
  std::vector<std::pair<uint32_t, std::unique_ptr<Rom>>> roms_;
};

}  // namespace gvm

#endif  // _GVM_COMPUTER_H_
