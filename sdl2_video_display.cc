#include "sdl2_video_display.h"

#include <cassert> 
#include <iostream>

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() : SDL2VideoDisplay(800, 600, true) {}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height)
  : SDL2VideoDisplay(width, height, false) {
}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height, const bool fullscreen)
  : buffer_(nullptr) {
  const auto flags = fullscreen
    ? SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN 
    : SDL_WINDOW_ALLOW_HIGHDPI;

  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
  window_ = SDL_CreateWindow("GVM", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
      width, height, flags);
  assert(window_ != nullptr);
  SDL_GetWindowSize(window_, &maxW_, &maxH_);
}

SDL2VideoDisplay::~SDL2VideoDisplay() {
  SDL_FreeSurface(buffer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void SDL2VideoDisplay::SetFramebufferSize(int fWidth, int fHeight, int bpp) {  
  assert(fWidth <= maxW_);
  assert(fHeight <= maxH_);
  assert(bpp == 32);
  fWidth_ = fWidth;
  fHeight_ = fHeight;
}

void SDL2VideoDisplay::CopyBuffer(uint32_t* mem) {
  std::cerr << "CopyBuffer\n";
  if (buffer_ == nullptr) {
    SDL_FreeSurface(buffer_);
    buffer_ = nullptr;
  }
  buffer_ = SDL_CreateRGBSurfaceFrom((void*)mem, fWidth_, fHeight_, 32, 4*fWidth_,
     0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000); 
  assert(buffer_ != nullptr);
}

void SDL2VideoDisplay::Render() {
  if (buffer_ == nullptr) return;
  if (SDL_BlitScaled(buffer_, nullptr, SDL_GetWindowSurface(window_), nullptr) != 0) {
    std::cerr << SDL_GetError() << std::endl;
    assert(false);
  }
  SDL_UpdateWindowSurface(window_);
}

bool SDL2VideoDisplay::CheckEvents() {
  SDL_Event event;
  return SDL_PollEvent(&event) == 1 && event.type == SDL_QUIT;
}
}  // namespace gvm
