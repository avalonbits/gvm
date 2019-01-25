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
  register uint32_t fps = 0;
  auto start = std::chrono::high_resolution_clock::now();
  std::cerr << mem_reg_ << std::endl;
  while (!shutdown_) {
    if (print_fps_) ++fps;
    const bool done = display_->CheckEvents();
    if (done) shutdown_ = done;

    if (mem_[mem_reg_] == 1) {
      std::cerr << "Copy mem.\n";
      display_->CopyBuffer(&mem_[mem_addr_]);
      mem_[mem_reg_] = 0;
    }
    display_->Render();
    if (print_fps_ && fps % 200 == 0) {
      const std::chrono::nanoseconds runtime =
          std::chrono::high_resolution_clock::now() - start;
      const auto time = runtime.count();
      std::cerr << "Avergate fps: " << (200 / (time / static_cast<double>(1000000000)))
                << "\n";
      start = std::chrono::high_resolution_clock::now();
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
