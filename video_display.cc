#include "video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace gvm {

VideoDisplay::VideoDisplay() {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  maxW_ = mode.width;
  maxH_ = mode.height;
  window_.reset(new sf::RenderWindow(mode, "GVM", sf::Style::Fullscreen));
}

VideoDisplay::VideoDisplay(int width, int height) {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  if (width > static_cast<int>(mode.width)) width = mode.width;
  if (height > static_cast<int>(mode.height)) height = mode.height;
  maxW_ = width;
  maxH_ = height;
  window_.reset(new sf::RenderWindow(sf::VideoMode(maxW_, maxH_, mode.bitsPerPixel), "GVM"));
}

VideoDisplay::~VideoDisplay() {
  if (window_ != nullptr && window_->isOpen()) {
    window_->close();
  }
}

void VideoDisplay::SetFramebufferSize(int fWidth, int fHeight, int bpp) {
  assert(fWidth <= maxW_);
  assert(fHeight <= maxH_);
  assert(bpp == 32);  // We only support 32bpp.
  w_scale_ = maxW_ / fWidth;
  h_scale_ = maxH_ / fHeight;
  texture.create(fWidth, fHeight);
  buffer_size_ = fWidth * fHeight * (bpp / 8);
  buffer_.reset(new uint8_t[buffer_size_]);
}

void VideoDisplay::Render(const uint32_t* mem) {
  std::memcpy(buffer_.get(), mem, buffer_size_);
  texture.update(buffer_.get());
  sf::Sprite sprite;
  sprite.setTexture(texture);
  sprite.scale(w_scale_, h_scale_);
  window_->clear();
  window_->draw(sprite);
  window_->display();
}

bool VideoDisplay::CheckEvents() {
  // Check all the window's events that were triggered since the
  // last iteration of the loop
  sf::Event event;
  while (window_->pollEvent(event)) {
    // "close requested" event: we close the window
    if (event.type == sf::Event::Closed) {
      window_->close();
      return true;
    }
  }
  return false;
}

}  // namespace gvm
