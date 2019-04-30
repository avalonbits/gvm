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

  const auto flags = fullscreen
      ? SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
      : SDL_WINDOW_ALLOW_HIGHDPI;
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cerr << SDL_GetError() << std::endl;
    assert(false);
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
  SDL_DestroyTexture(texture_);
  SDL_DestroyTexture(text_mode_texture_);
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
  text_mode_texture_ = SDL_CreateTexture(
      renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 768, 432);
  assert(texture_ != nullptr);
  assert(text_mode_texture_ != nullptr);
}

void SDL2VideoDisplay::CopyBuffer(uint32_t* mem) {
  int pitch;
  void* pixels = nullptr;
  if (SDL_LockTexture(texture_,  nullptr, &pixels, &pitch) != 0) {
    std::cerr << SDL_GetError() << "\n";
    assert(false);
  }
  assert(pixels != nullptr);
  std::memcpy(pixels, mem, pitch * fHeight_);
  SDL_UnlockTexture(texture_);
}

static void char2pixels(uint32_t* pixel, const uint32_t* from) {
}

void SDL2VideoDisplay::CopyTextBuffer(uint32_t* mem) {
  // Characters are 8x16 fixed size. That means for 768x432 pixels, we get 96x27 characters.
  // Each character requires 32 bits (4 bytes): 16 bits for char, 8 bits for foreground
  // color and 8 bits for background color.

  const uint32_t buf_size = 768*432;
  std::unique_ptr<uint32_t> pixels(new uint32_t[buf_size]);
  const uint32_t tbuf_xsize = 96;
  const uint32_t tbuf_ysize = 27;
  for (uint32_t y = 0; y < tbuf_ysize; ++y) {
    for (uint32_t x = 0; x < tbuf_xsize; ++x) {
      const uint32_t* from = &mem[y * tbuf_xsize + x];
      uint32_t* to = &pixels.get()[x*8 + y*16*768];
      char2pixels(to, from);
    }
  }
}

void SDL2VideoDisplay::Render() {
  if (SDL_RenderClear(renderer_) != 0) {
    std::cerr << "RendererClear: " << SDL_GetError() << std::endl;
  }
  if (SDL_RenderCopy(renderer_, texture_, nullptr, nullptr) != 0) {
    if (count_ % 20000 == 0) {
      std::cerr << "RenderCopy: " << SDL_GetError() << std::endl;
    }
    ++count_;
  }
  SDL_RenderPresent(renderer_);
}

bool SDL2VideoDisplay::CheckEvents() {
  return false;
}

}  // namespace gvm
