#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "cxxopts.hpp"
#include "isa.h"

int main(int argc, char* argv[]) {
  cxxopts::Options options("gvm", "A 32-bit virtual machine.");
  options.add_options()
      ("benchmark", "Run a benchmark with the loaded program.")
      ("runs", "Number of runs to execute the code.", cxxopts::value<uint32_t>())
      ;
  auto result = options.parse(argc, argv);

  sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
  window.setVerticalSyncEnabled(true);
  sf::CircleShape shape(360.f);
  shape.setFillColor(sf::Color::Green);
  window.clear();
  window.draw(shape);
  window.display();

  std::unique_ptr<gvm::CPU> cpu(new gvm::CPU());
  cpu->LoadProgram(0, {
      gvm::MovRI(12, -16),
      gvm::MovRI(13, 1),
      gvm::Call(0x7FF8),
      gvm::MovRI(1, 32),
      gvm::AddRR(0, 0, 1),
      gvm::MovRI(11, 0x1000),
      gvm::Jmp(0x3FE8)
  });

  cpu->LoadProgram(0x8000, {
      gvm::AddRR(12, 12, 13),
      gvm::Jne(-4),
      gvm::Ret(), 
  });

  cpu->LoadProgram(0x4000, {
      gvm::StorRR(14, 0),
      gvm::Jmp(-0x2004)
  });

  cpu->LoadProgram(0x2000, {
      gvm::StorRI(0x1004, 1),
      gvm::LoadRI(2, 0x1004),
      gvm::LoadRI(3, 0x1004),
      gvm::SubRR(4, 1, 0),
      gvm::MovRR(12, 4),
      gvm::AddRR(12, 12, 14),
      gvm::Halt()
  });

  // run the program as long as the window is open
  bool cpu_done = false;
  while (window.isOpen()) {
  	// check all the window's events that were triggered since the
    // last iteration of the loop
    sf::Event event;
    while (window.pollEvent(event)) {
      // "close requested" event: we close the window
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }
    cpu_done = cpu_done || cpu->Step();
  }
  std::cerr << cpu->PrintRegisters(/*hex=*/true);
  std::cerr << cpu->PrintMemory(0x1000, 0x1004);
  std::cerr << cpu->PrintStatusFlags();

  return 0;
}
