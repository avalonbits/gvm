#include <iostream>
#include <memory>

#include "cpu.h"
#include "isa.h"

int main(void) {
  std::unique_ptr<gvm::CPU> cpu(new gvm::CPU());
  cpu->LoadProgram(0, {
      gvm::MovRI(0, 16),
      gvm::MovRI(1, 32),
      gvm::AddRR(0, 0, 1),
      gvm::MovRR(2, 0),
      gvm::MovRR(3, 1),
      gvm::Halt()
  });
  cpu->Run();
  std::cerr << cpu->PrintRegisters();
}
