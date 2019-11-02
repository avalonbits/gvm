/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "video_controller.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>

#include "isa.h"

namespace gvm {

VideoController::VideoController(const bool print_fps, VideoDisplay* display)
  : print_fps_(false), display_(display), shutdown_(false) {
  assert(display != nullptr);
}

static int ReadInput(void* ptr) {
  InputController* input = reinterpret_cast<InputController*>(ptr);
  input->Read();
  return 0;
}

void VideoController::Run() {
  auto start = std::chrono::high_resolution_clock::now();
  display_->Render(1);
  SDL_Thread* input_thread = SDL_CreateThread(
      ReadInput, "ReadInput", reinterpret_cast<void*>(input_controller_.get()));
  ready_();
  while (!shutdown_) {
    signal_->recv();
    if (mem_[mem_reg_] == 0) {
      ready_();
      continue;
    }

    const uint32_t mode = mem_[mem_reg_];
    display_->CopyBuffer(&mem_[mem_addr_], mode);
    mem_[mem_reg_] = 0;
    ready_();

    if (print_fps_) start = std::chrono::high_resolution_clock::now();
    display_->Render(mode);

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
}

}  // namespace gvm
