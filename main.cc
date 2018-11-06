#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>

#include "cpu.h"
#include "isa.h"

int main(void) {
    std::unique_ptr<gvm::CPU> cpu(new gvm::CPU());

    cpu->LoadProgram({
        gvm::LoadRI(0, 16),
        gvm::LoadRI(1, 32),
        gvm::AddRR(0, 0, 1),
        gvm::LoadRR(2, 0),
        gvm::LoadRR(3, 1),
        gvm::Halt()
    });
    cpu->Run();
}
