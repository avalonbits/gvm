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

#ifndef _GVM_COMPUTER_H_
#define _GVM_COMPUTER_H_

#include <cassert>
#include <memory>
#include <utility>

#include "core.h"
#include "cpu.h"
#include "disk_controller.h"
#include "input_controller.h"
#include "memory_bus.h"
#include "rom.h"
#include "sync_types.h"
#include "timer.h"
#include "video_controller.h"

namespace gvm {

class Computer {
 public:
  // Owns cpu and video_display.
  Computer(CPU* cpu, Core<MemoryBus>* core, VideoController* video_controller,
           DiskController* disk_controller);

  // Takes ownership of rom.
  void LoadRom(const Rom* rom);

  void Run();
  void Shutdown();

 private:
  void RegisterVideoDMA();

  const uint32_t mem_size_bytes_;
  std::unique_ptr<uint32_t> mem_;
  std::unique_ptr<CPU> cpu_;
  std::unique_ptr<Core<MemoryBus>> core_;
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
