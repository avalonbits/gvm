#include "sdl2_video_display.h"

#include <cassert> 

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() {
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
}
SDL2VideoDisplay::SDL2VideoDisplay(int width, int height) {
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
}

}  // namespace gvm
