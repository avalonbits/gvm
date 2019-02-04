#include "input_controller.h"

#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>

#include "utf8.h"

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
  while (!shutdown_) {
    if (!SDL_WaitEventTimeout(&event, 10)) continue;

    // Event available
    switch (event.type) {
      case SDL_QUIT: {
        callback_(0xFFFFFFFF);
        Shutdown();
        break;
      }
      case SDL_KEYDOWN: {
        const uint32_t sym = event.key.keysym.sym;
        switch (sym) {
          case SDLK_LCTRL:
          case SDLK_RCTRL:
          case SDLK_LALT:
          case SDLK_RALT:
            callback_(sym);
            break;
          default:
            break;
        }
        break;
      }
      case SDL_KEYUP: {
        // use bit 29 to signify key up.
        const uint32_t sym = event.key.keysym.sym | 0x20000000;
        switch (sym) {
          case SDLK_LCTRL:
          case SDLK_RCTRL:
          case SDLK_LALT:
          case SDLK_RALT:
            callback_(sym);
            break;
          default:
            break;
        }
        break;
      }
      case SDL_TEXTINPUT: {
        const char* text = event.text.text;
        int codepoint = 0;
        utf8codepoint(text, &codepoint);
        callback_(static_cast<uint32_t>(codepoint));
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
