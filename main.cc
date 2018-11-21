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
  const uint32_t xinc = 640 * 4;
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
    gvm::Jeq(92),  // Halt

    // Load color to r3
    gvm::MovRI(3, 0xFF00),
    gvm::LslRI(3, 3, 16),
    gvm::OrrRI(3, 3, 0xFF),

    // Load framebuffer position to r2.
    gvm::AddRI(2, 2, 8*4),


    // Save r0, r1 and r2 to stack.
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 0),
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 1),
    gvm::SubRI(14, 14, 4),
    gvm::StorRR(14, 2),

    gvm::MovRI(0, 0x15),
    gvm::LslRI(0, 0, 16),
    gvm::OrrRI(0, 0, 0XFD04),

    gvm::Call(0x8000-80),

    // Copy back r0, r1, r2 from stack
    gvm::LoadRR(2, 14),
    gvm::AddRI(14, 14, 4),
    gvm::LoadRR(1, 14),
    gvm::AddRI(14, 14, 4),
    gvm::LoadRR(0, 14),
    gvm::AddRI(14, 14, 4),
    gvm::AddRI(0, 0, 4),  // Point to next char.
    gvm::Jmp(-96),

    gvm::Halt(),
  }));

  computer.LoadRom(user_offset + 0x8000, new gvm::Rom({
   // Let's pretend we called a function.

    // Decrease framebuffer addr.
    gvm::MovRI(10, 5),

    // Multiply r1 by 16 because each char uses 4 words.
    gvm::LslRI(1, 1, 4),
    gvm::SubRI(1, 1, 4),

    // Loop:
    gvm::SubRI(2, 2, xinc),
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
    gvm::AddRI(2, 2, xinc),

    // Start at x pos + 8
    gvm::MovRI(7, 8*4),

    // next position in framebuffer.
    gvm::SubRI(7, 7, 4),
    gvm::AddRI(13, 7, 4),
    gvm::Jeq(-16),  // Need to jump to next line

    gvm::AddRI(5, 5, 1),  // NextBit
    gvm::SubRI(6, 5, 32),  // Check we exausted word.
    gvm::Jeq(-56),  // Need to load the next char word. (Loop)

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
