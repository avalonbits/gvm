#ifndef _GVM_VIDEO_CONTROLLER_H_
#define _GVM_VIDEO_CONTROLLER_H_

#include <atomic>
#include <cstdint>

#include "video_display.h"

namespace gvm {

class VideoController {
 public:
  // Takes ownership of display.
  explicit VideoController(VideoDisplay* display);

  void RegisterDMA(uint32_t mem_reg, uint32_t mem_addr, int fWidth,
                   int fHeight, int bpp, uint32_t* mem);

  void Run();
  void Shutdown();

 private:
  uint32_t mem_reg_;
  uint32_t mem_addr_;
  uint32_t mem_size_bytes_;
  volatile uint32_t* mem_;
  std::unique_ptr<VideoDisplay> display_;
  std::atomic<bool> shutdown_;

};

}  // namespace gvm

#endif  // _GVM_VIDEO_CONTROLLER_H_

