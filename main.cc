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

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include "computer.h"
#include "core.h"
#include "cpu.h"
#include "cxxopts.hpp"
#include "disk.h"
#include "disk_controller.h"
#include "gfs.h"
#include "isa.h"
#include "memory_bus.h"
#include "null_video_display.h"
#include "sdl2_video_display.h"

gvm::VideoDisplay* CreateSDL2Display(const std::string& mode) {
  if (mode == "450p") {
    return new gvm::SDL2VideoDisplay(800, 450);
  } else if (mode == "480p") {
    return new gvm::SDL2VideoDisplay(854, 480);
  } else if (mode == "540p") {
    return new gvm::SDL2VideoDisplay(960, 540);
  } else if (mode == "720p") {
    return new gvm::SDL2VideoDisplay(1280, 720);
  } else if (mode == "900p") {
    return new gvm::SDL2VideoDisplay(1600, 900);
  } else if (mode == "1080p") {
    return new gvm::SDL2VideoDisplay(1920, 1080);
  } else if (mode == "fullscreen") {
    return new gvm::SDL2VideoDisplay();
  } else if (mode == "null") {
    return new gvm::NullVideoDisplay();
  }

  std::cerr << "No valid video mode provided. Defaulting to 720p.\n";
  return new gvm::SDL2VideoDisplay(1280, 720);
}

const gvm::Rom* ReadRom(const std::string& prgrom) {
  std::ifstream in(prgrom, std::ifstream::binary | std::ifstream::in);
  return gvm::Rom::FromFile(in);
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("gvm", "The GVM virtual machine.");
  options.add_options()
    ("prgrom", "Rom file used to boot computer. If present, will ignore chrom.",
               cxxopts::value<std::string>()->default_value(""))
    ("video_mode", "Video mode used. Values can be: null, fullscreen, 480p, "
                   "540p, 900p and 1080p",
                   cxxopts::value<std::string>()->default_value("900p"))
    ("disk_file", "File to be used as 1 GiB disk. If non-existent, will try to create.",
                  cxxopts::value<std::string>()->default_value(""))
    ;
  auto result = options.parse(argc, argv);

  const std::string disk_file = result["disk_file"].as<std::string>();
  std::unique_ptr<gvm::Disk> disk;
  if (!disk_file.empty()) {
    disk.reset(new gvm::FileBackedDisk(disk_file, 1 << 21));
    if (!disk->Init()) {
      std::cerr << "No disk available. All writes to it will fail.";
    }
    if (!gvm::gfs::IsDiskPartitioned(disk.get())) {
      std::cerr << "Need to partition disk.\n";
      gvm::gfs::PartitionTable table;
      memset(&table, 0, sizeof(gvm::gfs::PartitionTable));
      table.partitions[0].start_sector = 1;
      table.partitions[0].end_sector = (1 << 19) + 1;  // 512k sectors (256 MiB).
      table.set_partitions = 1;  // Just partition 0 is set.
      gvm::gfs::UCS2name(table.partitions[0].label, "root");
      if (!gvm::gfs::Partition(disk.get(), &table)) {
        std::cerr << "Disk was not partitioned!\n";
        return -1;
      }
    }
    if (!gvm::gfs::IsDiskPartitionFormated(disk.get(), 0)) {
      std::cerr << "Need to format disk.\n";
      if (!gvm::gfs::Format(disk.get(), 0)) {
        std::cerr << "Disk was not formated!\n";
        return -1;
      }
    }
  }

  auto* disk_controller = new gvm::DiskController({disk.get()});
  const std::string mode = result["video_mode"].as<std::string>();
  const bool print_fps = mode != "null";
  auto* display = CreateSDL2Display(mode);
  auto* video_controller = new gvm::VideoController(print_fps, display);
  auto* cpu = new gvm::CPU();
  auto* core = new gvm::Core<gvm::MemoryBus>();
  gvm::Computer computer(cpu, core, video_controller, disk_controller);
  const std::string prgrom = result["prgrom"].as<std::string>();
  const gvm::Rom* rom = nullptr;
  rom = ReadRom(prgrom);
  computer.LoadRom(rom);
  computer.Run();

  return 0;
}
