#include "sdl2_video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() : SDL2VideoDisplay(800, 600, true) {}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height)
  : SDL2VideoDisplay(width, height, false) {
}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height, const bool fullscreen)
  : texture_(nullptr) {
  const auto flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
  assert(SDL_Init(SDL_INIT_EVERYTHING) >= 0);
  window_ = SDL_CreateWindow("GVM", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
      width, height, flags);
  assert(window_ != nullptr);
  SDL_GetWindowSize(window_, &maxW_, &maxH_);

  renderer_ = SDL_CreateRenderer(
      window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer_ == nullptr) {
    std::cerr << "Hardware acceleration unavailable. Fallback to software.\n";
    renderer_ = SDL_CreateRenderer(
        window_, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
  }
  assert(renderer_ != nullptr);
}

SDL2VideoDisplay::~SDL2VideoDisplay() {
  SDL_DestroyTexture(texture_);
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

void SDL2VideoDisplay::Render() {
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
  SDL_RenderPresent(renderer_);
}

bool SDL2VideoDisplay::CheckEvents() {
  SDL_Event event;
  return SDL_PollEvent(&event) == 1 && event.type == SDL_QUIT;
}
}  // namespace gvm
