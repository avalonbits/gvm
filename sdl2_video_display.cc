/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sdl2_video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() : SDL2VideoDisplay(800, 600, true, "") {}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height)
  : SDL2VideoDisplay(width, height, false, "") {
    count_ = 0;
}

SDL2VideoDisplay::SDL2VideoDisplay(
    int width, int height, const bool fullscreen, const std::string force_driver)
  : texture_(nullptr) {

  // The text buffer is 100x28 chars wide, with each char uisng 4 bytes (2 for
  // char, 1 for fgcolor, 1 for bg color.
  text_vram_buffer_ = new uint32_t[100*28];
  text_pixels_ = new uint32_t[800*450];
  memset(text_vram_buffer_, 0, sizeof(uint32_t)*100*28);
  memset(text_pixels_, 0, sizeof(uint32_t)*800*450);

  const auto flags = fullscreen
      ? SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
      : SDL_WINDOW_ALLOW_HIGHDPI;
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cerr << SDL_GetError() << std::endl;
    assert(false);
  }
  if (fullscreen) {
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
      SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
    } else {
      width = dm.w;
      height = dm.h;
    }
  }
  window_ = SDL_CreateWindow("GVM", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
      width, height, flags);
  assert(window_ != nullptr);
  SDL_GetWindowSize(window_, &maxW_, &maxH_);


  int drv_index = -1;
  if (!force_driver.empty()) {
    SDL_Log("Available renderers:\n");

    for(int it = 0; it < SDL_GetNumRenderDrivers(); it++) {
      SDL_RendererInfo info;
      SDL_GetRenderDriverInfo(it,&info);
      SDL_Log("%s\n", info.name);
      if (force_driver == info.name) {
        drv_index = it;
        SDL_Log("picked opengles2");
        break;
      }
    }
    if (drv_index == -1) {
      SDL_Log("%s was not available. Letting SDL choose.", force_driver.c_str());
    }
  }

  renderer_ = SDL_CreateRenderer(window_, drv_index, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer_ == nullptr) {
    SDL_Log("%s", SDL_GetError());
    std::cerr << "Hardware acceleration unavailable. Fallback to software.\n";
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
  }
}

SDL2VideoDisplay::~SDL2VideoDisplay() {
  delete [] text_vram_buffer_;
  delete [] text_pixels_;
  SDL_DestroyTexture(texture_);
  SDL_DestroyTexture(text_texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void SDL2VideoDisplay::SetFramebufferSize(int fWidth, int fHeight, int bpp) {
  assert(fWidth <= maxW_);
  assert(fHeight <= maxH_);
  assert(bpp == 32);
  fWidth_ = fWidth;
  fHeight_ = fHeight;
  texture_ = SDL_CreateTexture(
      renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, fWidth, fHeight);
  assert(texture_ != nullptr);
  text_texture_ = SDL_CreateTexture(
      renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 800, 450);
  assert(text_texture_ != nullptr);
}

void SDL2VideoDisplay::CopyBuffer(uint32_t* mem, uint32_t mode) {
  if (mode == 1) {
    int pitch;
    void* pixels = nullptr;
    if (SDL_LockTexture(texture_,  nullptr, &pixels, &pitch) != 0) {
      std::cerr << SDL_GetError() << "\n";
      assert(false);
    }
    assert(pixels != nullptr);
    std::memcpy(pixels, mem, pitch * fHeight_);
    SDL_UnlockTexture(texture_);
  } else if (mode == 2) {
    std::memcpy(text_vram_buffer_, mem, sizeof(uint32_t)*100*28);
  }
}

void SDL2VideoDisplay::Render(uint32_t mode) {
  if (SDL_RenderClear(renderer_) != 0) {
    std::cerr << "RendererClear: " << SDL_GetError() << std::endl;
  }

  if (mode == 1) {
    GraphicsRender();
  } else if (mode == 2) {
    TextRender();
  }

  SDL_RenderPresent(renderer_);
}

void SDL2VideoDisplay::GraphicsRender() {
  if (SDL_RenderCopy(renderer_, texture_, nullptr, nullptr) != 0) {
    if (count_ % 20000 == 0) {
      std::cerr << "RenderCopy: " << SDL_GetError() << std::endl;
    }
    ++count_;
  }
}

static void renderChar(
    uint32_t ch, uint32_t fg, uint32_t bg, int x, int y,
    uint32_t* text_rom,  uint32_t* vram_pixels) {
    const uint32_t line_size = 800;
    const uint32_t char_width = 8;
    uint32_t* char_word = &text_rom[ch << 2];
    uint32_t idx = y * line_size * 16 + x * char_width - line_size;
    for (int w = 0; w < 4; ++w) {
        uint32_t word = char_word[w];
        for (uint32_t i = 0; i < sizeof(uint32_t)*char_width; ++i) {
            if (i % 8 == 0) {
                idx += line_size + char_width;
            }
            auto c = (word >> i) & 0x01;
            vram_pixels[--idx] = c == 0 ? bg : fg;
        }
    }
}

void SDL2VideoDisplay::TextRender() {
  for (int y = 0; y < 28; ++y) {
    for (int x = 0; x < 100; ++x) {
      const auto i = y*100 + x;
      const auto ch = text_vram_buffer_[i] & 0xFFFF;
      const auto fg = color_table_[(text_vram_buffer_[i] >> 16) & 0xFF];
      const auto bg = color_table_[(text_vram_buffer_[i] >> 24) & 0xFF];
      renderChar(ch, fg, bg, x, y, text_rom_, text_pixels_);
    }
  }

  int pitch;
  void* pixels = nullptr;
  if (SDL_LockTexture(text_texture_,  nullptr, &pixels, &pitch) != 0) {
    std::cerr << SDL_GetError() << "\n";
    assert(false);
  }
  assert(pixels != nullptr);
  std::memcpy(pixels, text_pixels_, pitch * 450);
  SDL_UnlockTexture(text_texture_);

  if (SDL_RenderCopy(renderer_, text_texture_, nullptr, nullptr) != 0) {
    if (count_ % 20000 == 0) {
      std::cerr << "RenderCopy: " << SDL_GetError() << std::endl;
    }
    ++count_;
  }
}

bool SDL2VideoDisplay::CheckEvents() {
  return false;
}

}  // namespace gvm
