#include "sdl2_video_display.h"

#include <cassert> 

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() : SDL2VideoDisplay(800, 600, true) {}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height)
  : SDL2VideoDisplay(width, height, false) {
}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height, const bool fullscreen) {
  const auto flags = fullscreen
    ? SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
    : SDL_WINDOW_ALLOW_HIGHDPI;

  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
  window_ = SDL_CreateWindow("GVM", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
      width, height, flags);
  assert(window_ != nullptr);
}

}  // namespace gvm
