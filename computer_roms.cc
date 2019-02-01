#include "computer_roms.h"

#include "isa.h"

namespace gvm {
namespace rom {

static const uint32_t kLineLength = 640 * 4;  // 640 pixels, 32bpp.

Rom* Textmode(uint32_t user_offset) {
  auto* rom = new Rom(user_offset + 0x3000, {
    // First the 16 colors that we support
    Word(0xFF000000),  // 0  - Black
    Word(0xFF000080),  // 1  - Maroon
    Word(0xFF008000),  // 2  - Green
    Word(0xFF008080),  // 3  - Olive
    Word(0xFF800000),  // 4  - Navy
    Word(0xFF800080),  // 5  - Purple
    Word(0xFF808000),  // 6  - Teal
    Word(0xFFC0C0C0),  // 7  - Silver
    Word(0xFF808080),  // 8  - Grey
    Word(0xFF0000FF),  // 9  - Red
    Word(0xFF00FF00),  // 10 - Lime
    Word(0xFF00FFFF),  // 11 - Yellow
    Word(0xFFFF0000),  // 12 - Blue
    Word(0xFFFF00FF),  // 13 - Fuchsia
    Word(0xFFFFFF00),  // 14 - Aqua
    Word(0xFFFFFFFF),  // 15 - White

    // Now for the funcions.
    //
    // ==================== writec ===========================================
    // Address: 0x103040
    SubRI(0, 29, 0x40),
    LslRI(3, 3, 2),
    AddRR(3, 0, 3),
    LoadRR(3, 3),

    // Set r0 to point to start address of character rom (0x104000)
    MovRI(0, 0x104),
    LslRI(0, 0, 12),

    // Decrease framebuffer addr.
    MovRI(10, 5),

    // Multiply r1 by 16 because each char uses 4 words.
    LslRI(1, 1, 4),
    SubRI(1, 1, 4),

    // Loop:
    SubRI(2, 2, kLineLength),
    SubRI(10, 10, 1),
    Jeq(10, 76),  // We are done.

    // Now add the result with r0 to point to the correct char.
    AddRI(1, 1, 4),
    AddRR(12, 0, 1),

    // Load the char word to r4
    LoadRR(4, 12),

    // Bits to shift right.
    MovRI(5, -1),

    // In word loop:
    // Move to the next line.
    AddRI(2, 2, kLineLength),

    // Start at x pos + 8
    MovRI(7, 8*4),

    // next position in framebuffer.
    SubRI(7, 7, 4),
    AddRI(12, 7, 4),
    Jeq(12, -16),  // Need to jump to next line

    AddRI(5, 5, 1),  // NextBit
    SubRI(6, 5, 32),  // Check we exausted word.
    Jeq(6, -56),  // Need to load the next char word. (Loop)

    LsrRR(6, 4, 5),   // shift char by r5 bits.
    AndRI(6, 6, 0x01), // Check if it is active.
    Jeq(6, -32),  // If not active, go to the next bit.

    AddRR(6, 2, 7),
    StorRR(6, 3),  // Store color in framebuffer
    Jmp(-44),
    Ret(),

    // ==================== writec ============================================

    // ==================== putc ==============================================
    // Address: 0xE24BE
    // Calls writec then signals that the buffer is ready.
    MovRI(0, 0xE24),
    LslRI(0, 0, 8),
    AddRI(0, 0, 0x50),
    CallR(0),

    // Signal to the buffer.
    MovRI(0, 1),
    StorRI(0x80, 0),
    Ret(),
    // ==================== putc ==============================================
  });

  rom->Load(user_offset + 0x2000, {
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
    gvm::Word(0x57),  // W
    gvm::Word(0x6F),  // o
    gvm::Word(0x72),  // r
    gvm::Word(0x6C),  // l
    gvm::Word(0x64),  // d
    gvm::Word(0x21),  // !
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
    gvm::Word(0x20),  // <space>
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
  });

  rom->Load(user_offset, {
    gvm::MovRI(20, 0x1000),
    // Set r0 to the mem start position of the string.
    gvm::MovRI(0, 1),
    gvm::LslRI(0, 0,  20),
    gvm::AddRI(0, 0, 0x2000),

    // Set r2 to the first position - 8 pixels.
    gvm::MovRI(2, 0x64),

    // Loop:
    gvm::LoadRR(1, 0),  // Get current char.
    gvm::Jeq(1, 84),  // If it's 0, we are done.

    // Load color to r3
    gvm::MovRI(3, 2),

    // Load framebuffer position to r2.
    gvm::AddRI(2, 2, 8*4),

    // Save r0, r1 and r2 to stack.
    gvm::SubRI(30, 30, 4),
    gvm::StorRR(30, 0),
    gvm::SubRI(30, 30, 4),
    gvm::StorRR(30, 1),
    gvm::SubRI(30, 30, 4),
    gvm::StorRR(30, 2),

    gvm::MovRI(0, 1),
    gvm::LslRI(0, 0, 20),
    gvm::AddRI(0, 0, 0x3040),
    gvm::CallR(0),

    // Copy back r0, r1, r2 from stack
    gvm::LoadRR(2, 30),
    gvm::AddRI(30, 30, 4),
    gvm::LoadRR(1, 30),
    gvm::AddRI(30, 30, 4),
    gvm::LoadRR(0, 30),
    gvm::AddRI(30, 30, 4),
    gvm::AddRI(0, 0, 4),  // Point to next char.
    gvm::Jmp(-84),

    gvm::MovRI(0, 1),
    gvm::StorRI(0x80, 0),
    gvm::SubRI(20, 20, 1),
    gvm::Jne(20, -116),
    gvm::Halt(),
  });

  // When CPU boots at reading from address 0, which is also the address of the
  // reset signal. Have it jump to 0x100000 which is the start of available
  // memory.
  rom->Load(0, {
    gvm::Jmp(0xE108C),  // Reset interrupt handler @0xE108C.
    gvm::Jmp(0xE109C),  // Timer interrupt handler @0xE10A0. Jump is pc relative.
  });

  rom->Load(0xE108C, {
    // For now we reset the timer counters and then jump to user code.
    gvm::MovRI(0, 0),
    gvm::StorRI(0xE1084, 0),
    gvm::StorRI(0xE1088, 0),
    gvm::Jmp(0x100000-0xE1098),  // Jump to user code.
  });

  rom->Load(0xE10A0, {
    // Implements a 64bit tick counter.

    // First, save contents of r0 so we don't disrupt user code.
    gvm::SubRI(30, 30, 4),
    gvm::StorRR(30, 0),

    // Load low 32 bits.
    gvm::LoadRI(0, 0xE1084),

    // Increment value.
    gvm::AddRI(0, 0, 1),

    // Store back.
    gvm::StorRI(0xE1084, 0),

    // If != 0 i.e no overflow, we are done. Otherwise, increment high 32 bits.
    gvm::Jne(0, 0x10),

    // Load high 32 bits.
    gvm::LoadRI(0, 0xE1088),

    // Increment value.
    gvm::AddRI(0, 0, 1),

    // Store back.
    gvm::StorRI(0xE1088, 0),

    // We are done. Load back r0 and return.
    gvm::LoadRR(0, 30),
    gvm::AddRI(30, 30, 4),
    gvm::Ret(),
  });

  return rom;
}

}  // namespace rom
}  // namespace gvm
