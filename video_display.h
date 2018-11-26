#ifndef _GVM_VIDEO_DISPLAY_H_
#define _GVM_VIDEO_DISPLAY_H_

#include <atomic>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gvm {

class VideoDisplay {
 public:
  VideoDisplay() {}
  virtual ~VideoDisplay() {}

  virtual void SetFramebufferSize(int fWidth, int fHeight, int bpp) = 0;
  virtual void CopyBuffer(const uint32_t* mem) = 0;
  virtual void Render() = 0;
  virtual bool CheckEvents() = 0;

};

}  // namespace gvm

#endif  // _GVM_VIDEO_DISPLAY_H_
