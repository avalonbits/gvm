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

#ifndef _GVM_VIDEO_DISPLAY_H_
#define _GVM_VIDEO_DISPLAY_H_

#include <atomic>
#include <memory>

namespace gvm {

class VideoDisplay {
 public:
  explicit VideoDisplay() {}
  virtual ~VideoDisplay() {}

  virtual void SetFramebufferSize(int fWidth, int fHeight, int bpp) = 0;
  virtual void SetTextRom(uint32_t* mem) = 0;
  virtual void SetColorTable(uint32_t* mem) = 0;
  virtual void CopyBuffer(uint32_t* mem, uint32_t mode) = 0;
  virtual void Render(uint32_t mode) = 0;
  virtual bool CheckEvents() = 0;
};

}  // namespace gvm

#endif  // _GVM_VIDEO_DISPLAY_H_
