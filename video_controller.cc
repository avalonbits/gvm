#include "video_controller.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "isa.h"

namespace gvm {

VideoController::VideoController(const bool print_fps, VideoDisplay* display)
  : print_fps_(print_fps), display_(display), shutdown_(false) {
  assert(display != nullptr);
}

static int ReadInput(void* ptr) {
  InputController* input = reinterpret_cast<InputController*>(ptr);
  input->Read();
  return 0;
}

void VideoController::Run() {
  auto start = std::chrono::high_resolution_clock::now();
  display_->Render();
  SDL_Thread* input_thread = SDL_CreateThread(
          ReadInput, "ReadInput", reinterpret_cast<void*>(input_controller_.get()));
  while (!shutdown_) {
    signal_->recv();
    if (mem_[mem_reg_] == 0) continue;

    display_->CopyBuffer(&mem_[mem_addr_]);
    mem_[mem_reg_] = 0;

    if (print_fps_) start = std::chrono::high_resolution_clock::now();
    display_->Render();

    if (print_fps_) {
      const std::chrono::nanoseconds runtime =
          std::chrono::high_resolution_clock::now() - start;
      const auto time = runtime.count();
      std::cout << "Avergate fps: " << (1 / (time / static_cast<double>(1000000000)))
                << "\n";
    }
  }
  input_controller_->Shutdown();
  int v;
 SDL_WaitThread(input_thread, &v);
}

void VideoController::RegisterDMA(
    uint32_t mem_reg, uint32_t mem_addr, int fWidth, int fHeight, int bpp,
    uint32_t* mem) {
  assert(mem != nullptr);
  mem_reg_ = mem_reg / sizeof(uint32_t);
  mem_addr_ = mem_addr;
  mem_ = mem;
  display_->SetFramebufferSize(fWidth, fHeight, bpp);
}

void VideoController::Shutdown() {
  shutdown_ = true;
  SDL_Event ev;
  ev.type = SDL_QUIT;
  SDL_PushEvent(&ev);
  signal_->Close(); 
  signal_->send();
}

}  // namespace gvm
