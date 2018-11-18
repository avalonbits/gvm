#include <cassert>
#include <cstdint>
#include <memory>
#include <iostream>
#include <SFML/Graphics.hpp>

int main(void) {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  sf::RenderWindow window(mode, "Texture", sf::Style::Fullscreen);

  int maxX = mode.width/2, maxY = mode.height/2;
  sf::Texture texture;
  assert(texture.create(maxX, maxY));

  uint32_t memsize = maxX * maxY;
  uint32_t* mem = new uint32_t[memsize];

  for (uint32_t i = 0; i < memsize; ++i) {
    mem[i] = 0xFF00FF00;
  }

  texture.update(reinterpret_cast<uint8_t*>(mem));
  sf::Sprite sprite;
  sprite.setTexture(texture);
  sprite.scale(2, 2);

  int i = 0;
  while (window.isOpen()) {
    std::cerr << i++ << std::endl;
    // check all the window's events that were triggered since the
    // last iteration of the loop
    sf::Event event;
    while (window.pollEvent(event)) {
      // "close requested" event: we close the window
      if (event.type == sf::Event::Closed) {
          window.close();
      }
    }
    window.clear();
    window.draw(sprite);
    window.display();
  }

  delete [] mem;
  return 0;
}
