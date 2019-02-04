#include "input_controller.h"

#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>

namespace gvm {

InputController::InputController(std::function<void(uint32_t value)> callback)
    : shutdown_(false), callback_(callback) {
  if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK) < 0) {
    std::cerr << SDL_GetError() << std::endl;
    assert(false);
  }
}

InputController::~InputController() {}

void InputController::Run() {
  SDL_Event event;
  bool ctrl_l = false;
  bool ctrl_r = false;
  while (!shutdown_) {
    if (!SDL_WaitEventTimeout(&event, 10)) continue;
    // Event available
    switch (event.type) {
      case SDL_QUIT: {
        callback_(1);
        Shutdown();
        break;
      }
      case SDL_KEYDOWN: {
        std::cerr << event.key.keysym.sym << std::endl;
        if (event.key.keysym.sym == SDLK_LCTRL) ctrl_l = true;
        else if (event.key.keysym.sym == SDLK_RCTRL) ctrl_r = true;

        if (event.key.keysym.sym == SDLK_q && (ctrl_l || ctrl_r)) {
          callback_(1);
          Shutdown();
        }
        break;
      }
      case SDL_KEYUP: {
        if (event.key.keysym.sym == SDLK_LCTRL) ctrl_l = false;
        if (event.key.keysym.sym == SDLK_RCTRL) ctrl_r = false;
        break;
      }

      default:
        break;
    }
  }
}

void InputController::Shutdown() {
  shutdown_ = true;
}
}  // namespace gvm
