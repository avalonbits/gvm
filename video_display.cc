#include "video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace gvm {

VideoDisplay::VideoDisplay() {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  maxX_ = mode.width;
  maxY_ = mode.height;
  window_.reset(new sf::RenderWindow(mode, "GVM", sf::Style::Fullscreen));
  config();
}

VideoDisplay::VideoDisplay(int x, int y) {
  sf::VideoMode mode = sf::VideoMode::getDesktopMode();
  if (x > static_cast<int>(mode.width)) x = mode.width;
  if (y > static_cast<int>(mode.height)) y = mode.height;
  maxX_ = x;
  maxY_ = y;
  window_.reset(new sf::RenderWindow(sf::VideoMode(x, y, mode.bitsPerPixel), "GVM"));
  config();
}

VideoDisplay::~VideoDisplay() {
  if (window_ != nullptr && window_->isOpen()) {
    window_->close();
  }
  delete [] mem_;
  delete [] buffer_;
}

void VideoDisplay::SendData(uint32_t* data, uint32_t byte_count) {
    assert(data != nullptr);
    assert(byte_count / sizeof(uint32_t) == mem_size_);
    std::memcpy(buffer_, data, byte_count);
    new_buffer_ = true;
}

void VideoDisplay::config() {
  std::cerr << "Window size: " << maxX_ << "x" << maxY_ << std::endl;
  window_->setVerticalSyncEnabled(true);

  // we want to fit 120 8x16 chars per line.
  text_scale_ = maxX_ / static_cast<double>(8.0) / static_cast<double>(120.0);
  mem_size_ = ((maxX_ / text_scale_) * (maxY_ / text_scale_));
  mem_ = new uint32_t[mem_size_];
  buffer_ = new uint32_t[mem_size_];
  std::memset(mem_, 0, mem_size_ * 4);
  std::memset(buffer_, 0, mem_size_ * 4);
  shutdown_ = false;
  new_buffer_ = false;
}

void VideoDisplay::RenderLoop() {
    int maxX = maxX_ / text_scale_, maxY = maxY_ / text_scale_;
    std::cerr << "Texture size: " << maxX << "x" << maxY << "(using text_scale=" << text_scale_ << ")\n";

    sf::Texture texture;
    assert(texture.create(maxX, maxY));
    texture.update(reinterpret_cast<uint8_t*>(mem_));
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.scale(text_scale_, text_scale_);
    while (!shutdown_.load() && window_->isOpen()) {
        if (new_buffer_.load()) {
            std::memcpy(mem_, buffer_, mem_size_ * 4);
            new_buffer_.exchange(false);
            texture.update(reinterpret_cast<uint8_t*>(mem_));
            sprite.setTexture(texture);
        }

        // check all the window's events that were triggered since the
        // last iteration of the loop
        sf::Event event;
        while (window_->pollEvent(event)) {
          // "close requested" event: we close the window
          if (event.type == sf::Event::Closed) {
              window_->close();
          }
        }

        window_->clear();
        window_->draw(sprite);
        window_->display();
    }
}

}  // namespace gvm
