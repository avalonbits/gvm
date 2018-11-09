#include <iostream>
#include <memory>

#include "cpu.h"
#include "isa.h"

int main(void) {
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
  cpu->Run();
  std::cerr << cpu->PrintRegisters(/*hex=*/true);
  std::cerr << cpu->PrintMemory(0x1000, 0x1004);
  std::cerr << cpu->PrintStatusFlags();
}
