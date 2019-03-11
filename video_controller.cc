#include "video_controller.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include "isa.h"

namespace gvm {

VideoController::VideoController(const bool print_fps, VideoDisplay* display)
  : print_fps_(print_fps), display_(display), shutdown_(false) {
  assert(display != nullptr);
}

void VideoController::Run() {
  auto shutdown = shutdown_;
  auto start = std::chrono::high_resolution_clock::now();
  display_->Render();
  while (!shutdown) {
    input_controller_->Read();
    shutdown = shutdown_;
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
}

}  // namespace gvm
