#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

#include "computer.h"
#include "computer_roms.h"
#include "cpu.h"
#include "cxxopts.hpp"
#include "isa.h"
#include "sdl2_video_display.h"
#include "null_video_display.h"

gvm::VideoDisplay* CreateSDL2Display(const std::string& mode) {
  if (mode == "480p") {
    return new gvm::SDL2VideoDisplay(854, 480);
  } else if (mode == "540p") {
    return new gvm::SDL2VideoDisplay(960, 540);
  } else if (mode == "720p") {
    return new gvm::SDL2VideoDisplay(1280, 720);
  } else if (mode == "900p") {
    return new gvm::SDL2VideoDisplay(1600, 900);
  } else if (mode == "1080p") {
    return new gvm::SDL2VideoDisplay(1920, 1080);
  } else if (mode == "fullscreen") {
    return new gvm::SDL2VideoDisplay();
  } else if (mode != "null") {
      std::cerr << mode << " is not a valid mode. Going with \"null\".\n";
  }

  return new gvm::NullVideoDisplay();
}

const gvm::Rom* CreateRom(const cxxopts::ParseResult& result);
const gvm::Rom* ReadRom(const std::string& prgrom) {
  std::ifstream in(prgrom, std::ifstream::binary | std::ifstream::in);
  return gvm::Rom::FromFile(in);
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("gvm", "The GVM virtual machine.");
  options.add_options()
    ("prgrom", "Rom file used to boot computer. If present, will ignore chrom.",
              cxxopts::value<std::string>()->default_value(""))
    ("chrom", "Rom file with 8x16 characters.",
              cxxopts::value<std::string>()->default_value("./latin1.chrom"))
    ("video_mode", "Video mode used. Values can be: null, fullscreen, 480p, "
                   "540p, 900p and 1080p",
                   cxxopts::value<std::string>()->default_value("720p"))
    ("shutdown_on_halt", "Shutdowns program when CPU halts,",
                         cxxopts::value<bool>()->default_value("true"))
    ;
  auto result = options.parse(argc, argv);

  const std::string mode = result["video_mode"].as<std::string>();
  const bool print_fps = mode != "null";
  auto* display = CreateSDL2Display(mode);
  auto* controller = new gvm::VideoController(print_fps, display);
  const uint32_t mem_size = 256 << 20;  // 256MiB
  auto* cpu = new gvm::CPU();
  gvm::Computer computer(mem_size, cpu, controller,
                         result["shutdown_on_halt"].as<bool>());
  const std::string prgrom = result["prgrom"].as<std::string>();
  const gvm::Rom* rom = nullptr;
  if (prgrom.empty()) {
    rom = CreateRom(result);
  } else {
    rom = ReadRom(prgrom);
  }
  computer.LoadRom(rom);
  computer.Run();

  return 0;
}

const gvm::Rom* CreateRom(const cxxopts::ParseResult& result) {
  const uint32_t user_offset = 1 << 20;

  gvm::Rom* rom = gvm::rom::Textmode(user_offset + 0x3000);

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
    gvm::Word(0x00),  // null
  });

  rom->Load(user_offset, {
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
    gvm::Halt(),
  });

  std::ifstream chrom(result["chrom"].as<std::string>(),
                      std::ios::binary | std::ios::ate);
  assert(chrom.is_open());
  const auto size = chrom.tellg();
  chrom.seekg(0, chrom.beg);
  gvm::Word* words = new gvm::Word[size/sizeof(gvm::Word)];
  chrom.read(reinterpret_cast<char*>(words), size);
  chrom.close();
  std::vector<gvm::Word> program(words, words + size/sizeof(gvm::Word));
  delete []words;
  rom->Load(user_offset + 0x4000, program);

  std::ofstream out("helloworld.rom", std::ios::binary);
  rom->ToFile(out);
  return rom;
}
