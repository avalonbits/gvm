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

#ifndef _GVM_SDL2_VIDEO_DISPLAY_H_
#define _GVM_SDL2_VIDEO_DISPLAY_H_

#include <SDL2/SDL.h>

#include "video_display.h"

namespace gvm {

class SDL2VideoDisplay : public VideoDisplay {
 public:
  SDL2VideoDisplay();  // Fullscreen display
  SDL2VideoDisplay(int width, int height);  // Windowed display.
  SDL2VideoDisplay(int width, int height, const bool fullscreen, const std::string force_driver);

  ~SDL2VideoDisplay() override;

  void SetFramebufferSize(int fWidth, int fHeight, int bpp) override;
  void SetTextRom(uint32_t* mem) override { text_rom_ = mem; }
  void SetColorTable(uint32_t* mem) override { color_table_ = mem; }
  void CopyBuffer(uint32_t* mem, uint32_t mode) override;
  void Render(uint32_t mode) override;
  bool CheckEvents() override;

 private:
  void GraphicsRender();
  void TextRender();

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  SDL_Texture* text_texture_;
  int maxW_;
  int maxH_;
  int fWidth_;
  int fHeight_;
  int count_;
  uint32_t* color_table_;
  uint32_t* text_rom_;
  uint32_t* text_vram_buffer_;
  uint32_t* text_pixels_;
};

}  // namespace gvm

#endif  // _GVM_SDL2_VIDEO_DISPLAY_H_
