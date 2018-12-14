#include "sfml_video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace gvm {

SFMLVideoDisplay::SFMLVideoDisplay() {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  maxW_ = mode.width;
  maxH_ = mode.height;
  window_.reset(new sf::RenderWindow(mode, "GVM", sf::Style::Fullscreen));
  window_->setActive();
  window_->clear();
  window_->display();
}

SFMLVideoDisplay::SFMLVideoDisplay(int width, int height) {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  if (width > static_cast<int>(mode.width)) width = mode.width;
  if (height > static_cast<int>(mode.height)) height = mode.height;
  maxW_ = width;
  maxH_ = height;
  window_.reset(new sf::RenderWindow(sf::VideoMode(maxW_, maxH_, mode.bitsPerPixel), "GVM"));
  window_->setActive();
  window_->clear();
  window_->display();
}

SFMLVideoDisplay::~SFMLVideoDisplay() {
  if (window_ != nullptr && window_->isOpen()) {
    window_->close();
  }
}

void SFMLVideoDisplay::SetFramebufferSize(int fWidth, int fHeight, int bpp) {
  assert(fWidth <= maxW_);
  assert(fHeight <= maxH_);
  assert(bpp == 32);  // We only support 32bpp.
  w_scale_ = static_cast<double>(maxW_) / fWidth;
  h_scale_ = static_cast<double>(maxH_) / fHeight;
  fWidth_ = fWidth;
  fHeight_ = fHeight;
  texture_.create(fWidth, fHeight);
}

void SFMLVideoDisplay::UpdateWindowSize(int wWidth, int wHeight) {
  maxW_ = wWidth;
  maxH_ = wHeight;
  w_scale_ = static_cast<double>(maxW_) / fWidth_;
  h_scale_ = static_cast<double>(maxH_) / fHeight_;
  sf::FloatRect visibleArea(0, 0, maxW_, maxH_);
  window_->setView(sf::View(visibleArea));
  Render();
}

void SFMLVideoDisplay::CopyBuffer(uint32_t* mem) {
  texture_.update(reinterpret_cast<uint8_t*>(mem));
}

void SFMLVideoDisplay::Render() {
  sf::Sprite sprite;
  sprite.setTexture(texture_);
  sprite.scale(w_scale_, h_scale_);
  window_->clear();
  window_->draw(sprite);
  window_->setActive();
  window_->display();
}

bool SFMLVideoDisplay::CheckEvents() {
  // Check all the window's events that were triggered since the
  // last iteration of the loop
  sf::Event event;
  window_->setActive();
  while (window_->pollEvent(event)) {
    // "close requested" event: we close the window
    if (event.type == sf::Event::Closed) {
      window_->close();
      return true;
    } else if (event.type == sf::Event::Resized) {
      UpdateWindowSize(event.size.width, event.size.height);
    } else if (event.type == sf::Event::TextEntered) {
      std::cerr <<  "Text unicode: " << event.text.unicode << std::endl;
    } else if (event.type == sf::Event::KeyPressed) {
      if (event.key.code == sf::Keyboard::Key::W && event.key.control) {
        window_->close();
        return true;
      }
    }
  }
  return false;
}

}  // namespace gvm
