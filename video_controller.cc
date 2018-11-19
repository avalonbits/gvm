#include "video_controller.h"

#include <cassert>

#include "isa.h"

namespace gvm {

VideoController::VideoController(VideoDisplay* display)
  : display_(display), shutdown_(false) {
  assert(display != nullptr);
}

void VideoController::Run() {
  while (!shutdown_.load()) {
    shutdown_ = display_->CheckEvents();
    if ((mem_[mem_reg_] ^ uint32_t(0x01)) == 0xFFFFFFFF) {
      display_->Render(&mem_[mem_addr_]);
      mem_[mem_reg_] = 0;
    }
  }
}

void VideoController::RegisterDMA(
    uint32_t mem_reg, uint32_t mem_addr, int fWidth, int fHeight, int bpp,
    uint32_t* mem) {
  assert(mem != nullptr);
  mem_reg_ = mem_reg;
  mem_addr_ = mem_addr_;
  mem_ = mem;
  display_->SetFramebufferSize(fWidth, fHeight, bpp);
}

void VideoController::Shutdown() {
  shutdown_ = true;
}


}  // namespace gvm
