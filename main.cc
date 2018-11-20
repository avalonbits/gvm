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

    // Load framebuffer position to r2.
    gvm::MovRI(2, 0x400),

    // Load collor to r3
    gvm::MovRI(3, 0xFF00),
    gvm::LslRI(3, 3, 16),
    gvm::OrrRI(3, 3, 0xFF),

    // Load 0x42 (capital B code) in to r1
    gvm::MovRI(1, 0x42),
    gvm::Call(user_offset+0x8000-32),
    gvm::Halt(),
  }));

  computer.LoadRom(user_offset + 0x8000, new gvm::Rom({
   // Let's pretend we called a function.

    // Decrease framebuffer addr.
    gvm::SubRI(2, 2, 0xC80),
    gvm::MovRI(10, 5),

    // Multiply r1 by 16 because each char uses 4 words.
    gvm::LslRI(1, 1, 4),
    gvm::SubRI(1, 1, 4),

    // Loop:
    gvm::SubRI(10, 10, 1),
    gvm::Jeq(76),  // We are done.

    // Now add the result with r0 to point to the correct char.
    gvm::AddRI(1, 1, 4),
    gvm::AddRR(13, 0, 1),

    // Load the char word to r4
    gvm::LoadRR(4, 13),

    // Bits to shift right.
    gvm::MovRI(5, -1),

    // In word loop:
    // Move to the next line.
    gvm::AddRI(2, 2, 0xC80),

    // Start at x pos + 8
    gvm::MovRI(7, 8*4),

    // next position in framebuffer.
    gvm::SubRI(7, 7, 4),
    gvm::AddRI(13, 7, 4),
    gvm::Jeq(-16),  // Need to jump to next line

    gvm::AddRI(5, 5, 1),
    gvm::SubRI(6, 5, 32),  // Check we exausted word.
    gvm::Jeq(-52),  // Need to load the next char word.

    gvm::LsrRR(6, 4, 5),   // shift char by r5 bits.
    gvm::AndRI(6, 6, 0x01), // Check if it is active.
    gvm::Jeq(-32),  // If not active, go to the next bit.
    gvm::AddRR(6, 2, 7),
    gvm::StorRR(6, 3),  // Store color in framebuffer
    gvm::Jmp(-44),

    // Singal the framebuffer that we are good.
    gvm::MovRI(0, 1),
    gvm::StorRI(0x00, 0),
    gvm::Ret(),
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
