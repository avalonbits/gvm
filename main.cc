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
  auto* cpu = new gvm::CPU(19800000, 60);
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
  gvm::Rom* rom = gvm::rom::Textmode(0xE2410);

  const uint32_t user_offset = 16 << 20;
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
    gvm::MovRI(0, 16),
    gvm::LslRI(0, 0,  20),
    gvm::AddRI(0, 0, 0x2000),

    // Set r2 to the first position - 8 pixels.
    gvm::MovRI(2, 0xFEE),
    gvm::LslRI(2, 2, 16),
    gvm::AddRI(2, 2, 0x33C0-8*4),

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

    gvm::MovRI(0, 0xE24),
    gvm::LslRI(0, 0, 8),
    gvm::AddRI(0, 0, 0x50),
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
    gvm::StorRI(0x20, 0),
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
  rom->Load(0xE1400, program);

  std::ofstream out("helloworld.rom", std::ios::binary);
  rom->ToFile(out);
  return rom;
}
