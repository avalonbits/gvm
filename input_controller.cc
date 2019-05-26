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


static bool IsControlKey(uint32_t sym) {
  switch(sym) {
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    case SDLK_LALT:
    case SDLK_RALT:
    case SDLK_RETURN:
    case SDLK_RETURN2:
    case SDLK_BACKSPACE:
    case SDLK_DELETE:
    case SDLK_DOWN:
    case SDLK_UP:
    case SDLK_RIGHT:
    case SDLK_LEFT:
    case SDLK_TAB:
    case SDLK_F1:
    case SDLK_F2:
    case SDLK_F3:
    case SDLK_F4:
    case SDLK_F5:
    case SDLK_F6:
    case SDLK_F7:
    case SDLK_F8:
    case SDLK_F9:
    case SDLK_F10:
    case SDLK_F11:
    case SDLK_F12:
    case SDLK_ESCAPE:
      return true;
  }
  return false;
}


void InputController::Read() {
  SDL_Event event;
  while (!shutdown_) {
    if (!SDL_WaitEvent(&event)) continue;

    // Event available
    switch (event.type) {
      case SDL_QUIT: {
        callback_(0xFFFFFFFF);
        shutdown_ = true;
        break;
      }
      case SDL_KEYDOWN: {
        const uint32_t sym = event.key.keysym.sym;
        std::cerr << "Keydown: " << sym << std::endl;
        if (IsControlKey(sym)) callback_(sym);
        break;
      }
      case SDL_KEYUP: {
        uint32_t sym = event.key.keysym.sym;
        std::cerr << "Keyup: " << sym << std::endl;
        if (IsControlKey(sym)) {
            sym |= (1 << 31);
            callback_(sym);
        }
        break;
      }

    /*
      case SDL_TEXTINPUT: {
        const char* text = event.text.text;
        int codepoint = 0;
        utf8codepoint(text, &codepoint);
        callback_(static_cast<uint32_t>(codepoint));
        break;
      }*/
      default:
        break;
    }
  }
}

}  // namespace gvm
