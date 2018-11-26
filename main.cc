#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include <SFML/Graphics.hpp>

#include "computer.h"
#include "computer_roms.h"
#include "cpu.h"
#include "isa.h"
#include "video_display.h"

int main(int argc, char* argv[]) {
  auto* display = new gvm::VideoDisplay(1920,1080);
  auto* controller = new gvm::VideoController(display);
  const uint32_t mem_size = 256 << 20;  // 256MiB
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(mem_size, cpu, controller);

  const uint32_t user_offset = 16 << 20;
  computer.LoadRom(user_offset + 0x1000, new gvm::Rom({
    gvm::Word(0x48),  // H
    gvm::Word(0x65),  // e
    gvm::Word(0x6C),  // l
    gvm::Word(0x6C),  // l
    gvm::Word(0x6F),  // o
    gvm::Word(0x2C),  //,
    gvm::Word(0x20),  // <space>
    gvm::Word(0x57),  // W
    gvm::Word(0x6F),  // o
    gvm::Word(0x72),  // r
    gvm::Word(0x6C),  // l
    gvm::Word(0x64),  // d
    gvm::Word(0x21),  // !
    gvm::Word(0x00),  // null
  }));

  computer.LoadRom(user_offset, new gvm::Rom({
    // Set r0 to the mem start position of the string.
    gvm::MovRI(0, 16),
    gvm::LslRI(0, 0,  20),
    gvm::AddRI(0, 0, 0x1000),

    // Set r2 to the first position - 8 pixels.
    gvm::MovRI(2, 0x400-(8*4)),

    // Loop:
    gvm::LoadRR(1, 0),  // Get current char.
    gvm::AddRI(1, 1, 0), // check it's not 0.
    gvm::Jeq(84),  // We are done.

    // Load color to r3
    gvm::MovRI(3, 2),

    // Load framebuffer position to r2.
    gvm::AddRI(2, 2, 8*4),

    // Save r0, r1 and r2 to stack.
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 0),
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 1),
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 2),

    gvm::MovRI(0, 0xE24),
    gvm::LslRI(0, 0, 8),
    gvm::AddRI(0, 0, 0x50),
    gvm::CallR(0),

    // Copy back r0, r1, r2 from stack
    gvm::LoadRR(2, 14),
    gvm::AddRI(14, 14, 4),
    gvm::LoadRR(1, 14),
    gvm::AddRI(14, 14, 4),
    gvm::LoadRR(0, 14),
    gvm::AddRI(14, 14, 4),
    gvm::AddRI(0, 0, 4),  // Point to next char.
    gvm::Jmp(-88),

    gvm::MovRI(0, 1),
    gvm::StorRI(0x00, 0),
    gvm::Halt(),
  }));

  computer.LoadRom(0xE2410, gvm::rom::Textmode());
  if (argc >= 2) {
    std::ifstream chrom(argv[1], std::ios::binary | std::ios::ate);
    assert(chrom.is_open());
    const auto size = chrom.tellg();
    chrom.seekg(0, chrom.beg);
    gvm::Word* words = new gvm::Word[size/sizeof(gvm::Word)];
    chrom.read(reinterpret_cast<char*>(words), size);
    chrom.close();
    computer.LoadRom(0xE1400, new gvm::Rom(words, size/sizeof(gvm::Word)));
  }

  computer.Run(argc >= 3);
  return 0;
}
