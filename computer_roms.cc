#include "computer_roms.h"

#include "isa.h"

namespace gvm {
namespace rom {

static const uint32_t kLineLength = 640 * 4;  // 640 pixels, 32bpp.
static const uint32_t kFrameBufferStart = 0x84;

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
    gvm::MovRI(1, 0),
    gvm::MovRI(2, 4),
    gvm::MovRI(3, 636),
    gvm::MovRI(4, 4),
    gvm::MovRI(5, 0xFF),
    gvm::LslRI(5, 5, 24),
    gvm::AddRI(5, 5, 0xFF),
    gvm::CallI(-0x1DF44),

    gvm::MovRI(1, 356),
    gvm::MovRI(2, 4),
    gvm::MovRI(3, 636),
    gvm::MovRI(4, 4),
    gvm::CallI(-0x1DF5C),

    gvm::MovRI(1, 0),
    gvm::MovRI(2, 4),
    gvm::MovRI(3, 356),
    gvm::MovRI(4, 4),
    gvm::CallI(-0x1DF28),

    gvm::MovRI(1, 636),
    gvm::MovRI(2, 4),
    gvm::MovRI(3, 356),
    gvm::MovRI(4, 4),
    gvm::CallI(-0x1DF40),

    gvm::MovRI(0, 1),
    gvm::StorRI(0x80, 0),

    // Loop until we halt.
    gvm::Wfi(),
    gvm::Jmp(-4),/*
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
    gvm::Halt(),*/
  });

  // When CPU boots it starts reading from address 0, which is also the address
  // of the reset signal. Here we add jumps to each of the interrupt handlers.
  rom->Load(0, {
    gvm::Jmp(0xE108C),  // Reset interrupt handler @0xE108C.
    gvm::Jmp(0xE109C),  // Timer interrupt handler @0xE10A0.
    gvm::Jmp(0xE10D0),  // Input interrupt handler @0xE10D8.
  });

  rom->Load(0xE108C, {
    // For now we reset the timer counters and then jump to user code.
    gvm::MovRI(0, 0),
    gvm::StorRI(0xE1084, 0),
    gvm::StorRI(0xE1088, 0),
    gvm::Jmp(0x1EF68),  // Jump to user code.
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

  rom->Load(0xE10D8, {
    // First, save contents of r0 so we don't disrupt user code.
    gvm::SubRI(30, 30, 4),
    gvm::StorRR(30, 0),

    // Now read the value from the input.
    gvm::LoadRI(0, 0xE10D4),

    // If it is quit, then r0 will be 0xFFFFFFFF. Add 1 to check.
    gvm::AddRI(0, 0, 1),

    // Check if it is not zero.
    gvm::Jne(0, 8),

    // It was zero, so halt the cpu.
    gvm::Halt(),

    // It wasn't zero, so restore r0 and return.
    gvm::LoadRR(0, 30),
    gvm::AddRI(30, 30, 4),
    gvm::Ret(),
  });

  rom->Load(0xE20D4, {
    // hline: draws a horizontal line.
    // r1: y-pos
    // r2: x-start
    // r3: x-end
    // r4: width
    // r5: color (RGBA)

    // Multiply y-pos by kLineLength to get y in the frame buffer.
    gvm::MulRI(1, 1, kLineLength),

    // Multiply x-start and x-end by 4 for pixel size.
    gvm::LslRI(3, 3, 2),

    // Width loop:
    gvm::LslRI(8, 2, 2),

    // Now add mem start, x-start with y-pos to get the framebuffer start point.
    gvm::AddRR(7, 1, 8),
    gvm::AddRI(7, 7, kFrameBufferStart),

    // Line loop:
    // Write the pixel at the location.
    gvm::StorRR(7, 5),

    // Increment x-start.
    gvm::AddRI(8, 8, 4),

    // Check if we got to x-end.
    gvm::SubRR(6, 3, 8),

    // If equal, check width.
    gvm::Jeq(6, 12),

    // Increment framebuffer.
    gvm::AddRI(7, 7, 4),
    gvm::Jmp(-20),

    // Done with one line.
    gvm::SubRI(4, 4, 1),

    // If width == 0, we are done.
    gvm::Jle(4, 12),

    // Jump to next line and start again.
    gvm::AddRI(1, 1, kLineLength),
    gvm::Jmp(-48),

    // We are done.
    gvm::Ret(),
  });

  rom->Load(0xE2118, {
    // vline: draws a vertical line.
    // r1: x-pos
    // r2: y-start
    // r3: y-end
    // r4: width
    // r5: color (RGBA)

    // Multiply x-pos by 4 to get x in the frame buffer.
    gvm::LslRI(1, 1, 2),

    // Multiply y-start and y-end by kLineLength to get their positions.
    gvm::MulRI(3, 3, kLineLength),

    // Width loop:
    gvm::MulRI(8, 2, kLineLength),

    // Now add mem start, x-pos, y-start with y-end to get the framebuffer start point.
    gvm::AddRR(7, 1, 8),
    gvm::AddRI(7, 7, kFrameBufferStart),

    // Line loop:
    // Write the pixel at the location.
    gvm::StorRR(7, 5),

    // Increment y-start.
    gvm::AddRI(8, 8, kLineLength),

    // Check if we got to y-end.
    gvm::SubRR(6, 3, 8),

    // If equal, check width.
    gvm::Jeq(6, 12),

    // Increment framebuffer.
    gvm::AddRI(7, 7, kLineLength),
    gvm::Jmp(-20),

    // Done with one line.
    gvm::SubRI(4, 4, 1),

    // If width == 0, we are done.
    gvm::Jle(4, 12),

    // Jump to next line and start again.
    gvm::AddRI(1, 1, 4),
    gvm::Jmp(-48),

    // We are done.
    gvm::Ret(),

  });

  return rom;
}

}  // namespace rom
}  // namespace gvm
