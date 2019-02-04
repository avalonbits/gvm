#include "input_controller.h"

#include <SDL2/SDL.h>

namespace gvm {

InputController::InputController(std::function<void(uint32_t value)> callback)
    : shutdown_(false), callback_(callback) {}

InputController::~InputController() {}

void InputController::Run() {
  SDL_Event event;
  while (!shutdown_) {
    if (!SDL_WaitEventTimeout(&event, 10)) continue;

    // Event available.
    if (event.type == SDL_QUIT) {
      callback_(1);
      Shutdown();
    }
  }
}

void InputController::Shutdown() {
  shutdown_ = true;
}
}  // namespace gvm
