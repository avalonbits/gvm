#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include "computer.h"
#include "cpu.h"
#include "cxxopts.hpp"
#include "disk.h"
#include "gfs.h"
#include "isa.h"
#include "null_video_display.h"
#include "sdl2_video_display.h"

gvm::VideoDisplay* CreateSDL2Display(const std::string& mode) {
  if (mode == "480p") {
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
  if (!disk_file.empty()) {
    gvm::FileBackedDisk disk(disk_file, 1 << 21);
    if (!disk.Init()) {
      std::cerr << "No disk available. All writes to it will fail.";
    }
    gvm::gfs::DiskPartition partitions[4] = {
      {1, (1<<19)+1, "root"},
      {0, 0, ""},
      {0, 0, ""},
      {0, 0, ""},
    };
    if (!gvm::gfs::Partition(&disk, partitions)) {
      std::cerr << "Disk was not partitioned!";
      return -1;
    }
    if (!gvm::gfs::Format(&disk, 0)) {
      std::cerr << "Disk was not formated!";
      return -1;
    }
    if (!disk.Fsync()) {
      return -1;
    }
  }

  const std::string mode = result["video_mode"].as<std::string>();
  const bool print_fps = mode != "null";
  auto* display = CreateSDL2Display(mode);
  auto* controller = new gvm::VideoController(print_fps, display);
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(cpu, controller);
  const std::string prgrom = result["prgrom"].as<std::string>();
  const gvm::Rom* rom = nullptr;
  rom = ReadRom(prgrom);
  computer.LoadRom(rom);
  computer.Run();

  return 0;
}
