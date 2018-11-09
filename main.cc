#include <iostream>
#include <memory>

#include "cpu.h"
#include "isa.h"

int main(void) {
  std::unique_ptr<gvm::CPU> cpu(new gvm::CPU());
  cpu->LoadProgram(0, {
      gvm::MovRI(0, -16),
      gvm::MovRI(1, 32),
      gvm::AddRR(0, 0, 1),
      gvm::MovRI(14, 0x1000),
      gvm::Jmp(0x1FF0)
  });

  cpu->LoadProgram(0x2000, {
      gvm::StorRR(14, 0),
      gvm::StorRI(0x1004, 1),
      gvm::LoadRI(2, 0x1004),
      gvm::LoadRI(3, 0x1000),
      gvm::SubRR(4, 1, 0),
      gvm::MovRR(15, 4),
      gvm::AddRR(15, 15, 0),
      gvm::Halt()
  });
  cpu->Run();
  std::cerr << cpu->PrintRegisters();
  std::cerr << cpu->PrintMemory(0x1000, 0x1004);
}
