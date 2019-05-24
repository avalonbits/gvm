#ifndef _GVM_COMPUTER_H_
#define _GVM_COMPUTER_H_

#include <cassert>
#include <memory>
#include <utility>

#include "cpu.h"
#include "disk_controller.h"
#include "input_controller.h"
#include "rom.h"
#include "sync_types.h"
#include "timer.h"
#include "video_controller.h"

namespace gvm {

class Computer {
 public:
  // Owns cpu and video_display.
  Computer(CPU* cpu, VideoController* video_controller, DiskController* disk_controller);

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
  std::unique_ptr<InputController> input_controller_;
  std::unique_ptr<DiskController> disk_controller_;
  SyncPoint video_signal_;
  SyncChan<uint32_t> timer_chan_;
  std::unique_ptr<TimerService> timer_service_;
  SyncChan<uint32_t> timer2_chan_;
  std::unique_ptr<TimerService> timer2_service_;

};

}  // namespace gvm

#endif  // _GVM_COMPUTER_H_
