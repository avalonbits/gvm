#ifndef _GVM_NULL_VIDEO_DISPLAY_H_
#define _GVM_NULL_VIDEO_DISPLAY_H_

#include "video_display.h"

namespace gvm {

class NullVideoDisplay : public VideoDisplay {
 public:
   NullVideoDisplay() : VideoDisplay() {}
   ~NullVideoDisplay() override {}

  void SetFramebufferSize(int fWidth, int fHeight, int bpp) override {}
  void CopyBuffer(uint32_t* mem) override {}
  void Render() override {}
  bool CheckEvents() override { return false; }
};

}  // namesapce gvm

#endif  // _GVM_NULL_VIDEO_DISPLAY_H_
