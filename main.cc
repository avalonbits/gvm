#include <iostream>
#include <memory>

#include <SFML/Graphics.hpp>

#include "computer.h"
#include "cpu.h"
#include "cxxopts.hpp"
#include "isa.h"
#include "video_display.h"

int main(int argc, char* argv[]) {
  cxxopts::Options options("gvm", "A 32-bit virtual machine.");
  options.add_options()
      ("benchmark", "Run a benchmark with the loaded program.")
      ("runs", "Number of runs to execute the code.", cxxopts::value<uint32_t>())
      ;
  auto result = options.parse(argc, argv);
  auto* display = new gvm::VideoDisplay(1280, 720);
  auto* controller = new gvm::VideoController(display);
  const uint32_t mem_size = 256 << 20;  // 256MiB
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(mem_size, cpu, controller);

  const uint32_t user_offset = 16 << 20;
  computer.LoadRom(user_offset, new gvm::Rom({
      gvm::MovRI(0, 0xFF),
      gvm::LslRI(0, 0, 24),
      gvm::OrrRI(0, 0, 0xFF),
      gvm::StorRI(0x400, 0),
      gvm::StorRI(0x404, 0),
      gvm::StorRI(0x408, 0),
      gvm::StorRI(0x40C, 0),
      gvm::StorRI(0x410, 0),
      gvm::StorRI(0x414, 0),
      gvm::StorRI(0x418, 0),
      gvm::StorRI(0x40C, 0),
      gvm::StorRI(0x898, 0),
      gvm::StorRI(0x89C, 0),
      gvm::StorRI(0x8A0, 0),
      gvm::StorRI(0x8A4, 0),
      gvm::StorRI(0x8A8, 0),
      gvm::StorRI(0x8AC, 0),
      gvm::StorRI(0x8B0, 0),
      gvm::StorRI(0x8B4, 0),
      gvm::MovRI(0, 1),
      gvm::StorRI(0, 0),
      gvm::Halt()
  }));

  computer.Run();
  return 0;
}
