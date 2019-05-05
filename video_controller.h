#ifndef _GVM_VIDEO_CONTROLLER_H_
#define _GVM_VIDEO_CONTROLLER_H_

#include <cstdint>

#include "input_controller.h"
#include "sync_types.h"
#include "video_display.h"

namespace gvm {

class VideoController {
 public:
  // Takes ownership of display.
  VideoController(const bool print_fps, VideoDisplay* display);

  void RegisterDMA(uint32_t mem_reg, uint32_t mem_addr, int fWidth,
                   int fHeight, int bpp, uint32_t* mem);
  void SetInputController(InputController* input_controller) {
    input_controller_.reset(input_controller);
  }
  void SetSignal(SyncPoint* signal) { signal_ = signal; }
  void SetTextRom(uint32_t* mem) { display_->SetTextRom(mem); }
  void SetColorTable(uint32_t* mem) { display_->SetColorTable(mem); }
  void Run();
  void Shutdown();

 private:
  const bool print_fps_;
  SyncPoint* signal_;
  uint32_t mem_reg_;
  uint32_t mem_addr_;
  uint32_t mem_size_bytes_;
  uint32_t* mem_;
  std::unique_ptr<VideoDisplay> display_;
  std::unique_ptr<InputController> input_controller_;
  volatile bool shutdown_;

};

}  // namespace gvm

#endif  // _GVM_VIDEO_CONTROLLER_H_

