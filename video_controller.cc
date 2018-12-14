#include "video_controller.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include "isa.h"

namespace gvm {

VideoController::VideoController(VideoDisplay* display)
  : display_(display), shutdown_(false) {
  assert(display != nullptr);
}

void VideoController::Run() {
  while (!shutdown_.load()) {
    const bool done = shutdown_.exchange(display_->CheckEvents());
    if (done) shutdown_ = done;
    if (mem_[mem_reg_] == 1) {
      const auto start = std::chrono::high_resolution_clock::now();
      display_->CopyBuffer(&mem_[mem_addr_]);
      display_->Render();
      mem_[mem_reg_] = 0;
      const auto runtime = std::chrono::high_resolution_clock::now() - start;

      const auto time = runtime.count();
      std::cerr << "Copy + render runtime: " << (time / static_cast<double>(1000)) << "us\n";
    }
  }
}

void VideoController::RegisterDMA(
    uint32_t mem_reg, uint32_t mem_addr, int fWidth, int fHeight, int bpp,
    uint32_t* mem) {
  assert(mem != nullptr);
  mem_reg_ = mem_reg;
  mem_addr_ = mem_addr;
  mem_ = mem;
  display_->SetFramebufferSize(fWidth, fHeight, bpp);
}

void VideoController::Shutdown() {
  shutdown_ = true;
}


}  // namespace gvm
