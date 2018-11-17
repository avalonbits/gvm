#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

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
  gvm::VideoDisplay display(1280, 720);

  std::unique_ptr<gvm::CPU> cpu(new gvm::CPU());
  cpu->LoadProgram(0, {
      gvm::MovRI(12, -16),
      gvm::MovRI(13, 1),
      gvm::Call(0x7FF8),
      gvm::MovRI(1, 32),
      gvm::AddRR(0, 0, 1),
      gvm::MovRI(11, 0x1000),
      gvm::Jmp(0x3FE8)
  });

  cpu->LoadProgram(0x8000, {
      gvm::AddRR(12, 12, 13),
      gvm::Jne(-4),
      gvm::Ret(),
  });

  cpu->LoadProgram(0x4000, {
      gvm::StorRR(14, 0),
      gvm::Jmp(-0x2004)
  });

  cpu->LoadProgram(0x2000, {
      gvm::StorRI(0x1004, 1),
      gvm::LoadRI(2, 0x1004),
      gvm::LoadRI(3, 0x1004),
      gvm::SubRR(4, 1, 0),
      gvm::MovRR(12, 4),
      gvm::AddRR(12, 12, 14),
      gvm::Halt()
  });

  display.RenderLoop();

  std::cerr << cpu->PrintRegisters(/*hex=*/true);
  std::cerr << cpu->PrintMemory(0x1000, 0x1004);
  std::cerr << cpu->PrintStatusFlags();

  return 0;
}
