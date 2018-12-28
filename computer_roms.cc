#include "computer_roms.h"

#include "isa.h"

namespace gvm {
namespace rom {

static const uint32_t kLineLength = 640 * 4;  // 640 pixels, 32bpp.

const Rom* Textmode() {
  return new Rom({
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
    // Address: 0xE2450
    SubRI(0, 13, 64),
    LslRI(3, 3, 2),
    AddRR(3, 0, 3),
    LoadRR(3, 3),

    // Set r0 to point to start address of character rom
    MovRI(0, 0xE14),
    LslRI(0, 0, 8),

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
    StorRI(0x00, 0),
    Ret(),
    // ==================== putc ==============================================
  });
}

}  // namespace rom
}  // namespace gvm
