#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include <SFML/Graphics.hpp>

#include "computer.h"
#include "cpu.h"
#include "isa.h"
#include "video_display.h"

int main(int argc, char* argv[]) {
  auto* display = new gvm::VideoDisplay(1280, 720);
  auto* controller = new gvm::VideoController(display);
  const uint32_t mem_size = 256 << 20;  // 256MiB
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(mem_size, cpu, controller);

  const uint32_t user_offset = 16 << 20;
  computer.LoadRom(user_offset, new gvm::Rom({
    // Load 0x15FD04 into r0
    gvm::MovRI(0, 0x15),
    gvm::LslRI(0, 0, 16),
    gvm::OrrRI(0, 0, 0xFD04),

    // Load 0x41 (capital A code) in to r1
    gvm::MovRI(1, 0x41),

    // Load framebuffer position to r2.
    gvm::MovRI(2, 0x400),

    // Load collor to r3
    gvm::MovRI(3, 0xFF00),
    gvm::LslRI(3, 3, 16),
    gvm::OrrRI(3, 3, 0xFF),

    // Let's pretend we called a function.

    // Multiply r1 by 16 because each char uses 4 words.
    gvm::LslRI(1, 1, 4),

    // Now add the result with r0 to point to the correct char.
    gvm::AddRR(0, 0, 1),

    // Load the char word to r4
    gvm::LoadRR(4, 0),

    // Decrease framebuffer addr.
    gvm::SubRI(2, 2, 4),

    // Loop:
    // Bits to shift right.
    gvm::MovRI(5, -1),

    // In word loop:
    gvm::AddRI(2, 2, 4),   // Next framebuffer position
    gvm::AddRI(5, 5, 1),   // bits to shift increase.
    gvm::SubRI(6, 5, 32),  // Check we exausted word.
    gvm::Jeq(24),  // call halt.

    gvm::LsrRR(6, 4, 5),   // shift char by r5 bits.
    gvm::AndRI(6, 6, 0x01), // Check if it is active.
    gvm::Jeq(-24),  // If not active, go to the next bit.
    gvm::StorRR(2, 3),  // Store color in framebuffer
    gvm::Jmp(-32),

    // Singal the framebuffer that we are good.
    gvm::MovRI(0, 1),
    gvm::StorRI(0x00, 0),
    gvm::Halt(),
  }));

  if (argc >= 2) {
    std::ifstream chrom(argv[1], std::ios::binary | std::ios::ate);
    assert(chrom.is_open());
    const auto size = chrom.tellg();
    chrom.seekg(0, chrom.beg);
    gvm::Word* words = new gvm::Word[size/sizeof(gvm::Word)];
    chrom.read(reinterpret_cast<char*>(words), size);
    chrom.close();
    computer.LoadRom(0x15FD04, new gvm::Rom(words, size/sizeof(gvm::Word)));
  }

  computer.Run(argc >= 3);
  return 0;
}
