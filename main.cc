#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include "computer.h"
#include "cpu.h"
#include "cxxopts.hpp"
#include "isa.h"
#include "sdl2_video_display.h"
#include "null_video_display.h"

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
  } else if (mode != "null") {
      std::cerr << mode << " is not a valid mode. Going with \"null\".\n";
  }

  return new gvm::NullVideoDisplay();
}

const gvm::Rom* CreateRom(const cxxopts::ParseResult& result);
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
                   cxxopts::value<std::string>()->default_value("720p"))
    ;
  auto result = options.parse(argc, argv);

  const std::string mode = result["video_mode"].as<std::string>();
  const bool print_fps = mode != "null";
  auto* display = CreateSDL2Display(mode);
  auto* controller = new gvm::VideoController(print_fps, display);
  const uint32_t mem_size = 17 << 20;  // 17MiB
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(mem_size, cpu, controller);
  const std::string prgrom = result["prgrom"].as<std::string>();
  const gvm::Rom* rom = nullptr;
  rom = ReadRom(prgrom);
  computer.LoadRom(rom);
  computer.Run();

  return 0;
}
