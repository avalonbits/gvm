#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>

int toInt(const char c) {
  if (c >= 'A') {
    return c - 'A' + 10;
  }
  return c - '0';
}

int main(int argc, char* argv[]) {

  std::ifstream hexfile(argv[1]);
  std::ofstream binfile(argv[2], std::ios::binary);
  std::string line;

  sf::VideoMode mode = sf::VideoMode(1920, 1080);
  int maxX = mode.width;
  double scale = static_cast<double>(maxX) / 8 / 80;
  sf::RenderWindow window(mode,  "SFML works!");
  window.setVerticalSyncEnabled(true);

  int x = 0, y = 0;
  sf::RectangleShape rectangle;
  rectangle.setSize(sf::Vector2f(scale, scale));
  rectangle.setFillColor(sf::Color::Red);
  while (std::getline(hexfile, line)) {
    auto pos = line.find(":");
    assert(pos != std::string::npos);
    const char* str = &line[pos+1];
    uint32_t word = 0;
    int w = 0;
    for (int i = 0; i < 16; i++) {
      const int byte = ((toInt(str[2*i]) << 4) | toInt(str[2*i+1])) & 0xFF;
      word = word | (byte << w);
      w += 8;
      if (w == 32) {
        binfile.write(reinterpret_cast<char*>(&word), 4);
        w = 0;
        word = 0;
      }
      for (int j = 0; j < 8; j++) {
        if ((byte >> j) & 1) {
          rectangle.setPosition(x + (scale*8-1)-(scale*j), y + scale*i);
          window.draw(rectangle);
        }
      }
    }
    std::cerr << std::endl;
    x += (8*scale);
    if (x >= maxX) {
      x = 0;
      y += (16*scale);
    }
  }

  window.display();

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
  }

  binfile.close();
}
