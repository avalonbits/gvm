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

  computer.LoadRom(0, new gvm::Rom({
      gvm::MovRI(12, -16),
      gvm::MovRI(13, 1),
      gvm::Call(0x7FF8),
      gvm::MovRI(1, 32),
      gvm::AddRR(0, 0, 1),
      gvm::MovRI(11, 0x1000),
      gvm::Jmp(0x3FE8)
  }));

  computer.LoadRom(0x8000, new gvm::Rom({
      gvm::AddRR(12, 12, 13),
      gvm::Jne(-4),
      gvm::Ret(),
  }));

  computer.LoadRom(0x4000, new gvm::Rom({
      gvm::StorRR(14, 0),
      gvm::Jmp(-0x2004)
  }));

  computer.LoadRom(0x2000, new gvm::Rom({
      gvm::StorRI(0x1004, 1),
      gvm::LoadRI(2, 0x1004),
      gvm::LoadRI(3, 0x1004),
      gvm::SubRR(4, 1, 0),
      gvm::MovRR(12, 4),
      gvm::AddRR(12, 12, 14),
      gvm::Halt()
  }));

  computer.Run();
  return 0;
}
