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

#ifndef _GVM_NULL_VIDEO_DISPLAY_H_
#define _GVM_NULL_VIDEO_DISPLAY_H_

#include "video_display.h"

namespace gvm {

class NullVideoDisplay : public VideoDisplay {
 public:
   NullVideoDisplay() : VideoDisplay() {}
   ~NullVideoDisplay() override {}

  void SetFramebufferSize(int fWidth, int fHeight, int bpp) override {}
  void CopyBuffer(uint32_t* mem, uint32_t mode) override {}
  void SetTextRom(uint32_t* mem) override {}
  void SetColorTable(uint32_t* mem) override {}
  void Render(uint32_t mode) override {}
  bool CheckEvents() override { return false; }
};

}  // namesapce gvm

#endif  // _GVM_NULL_VIDEO_DISPLAY_H_
