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
