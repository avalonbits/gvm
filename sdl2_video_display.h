#ifndef _GVM_SDL2_VIDEO_DISPLAY_H_
#define _GVM_SDL2_VIDEO_DISPLAY_H_

#include <SDL2/SDL.h>

#include "video_display.h"

namespace gvm {

class SDL2VideoDisplay : public VideoDisplay {
 public:
  SDL2VideoDisplay();  // Fullscreen display
  SDL2VideoDisplay(int width, int height);  // Windowed display.
  ~SDL2VideoDisplay() override;

  void SetFramebufferSize(int fWidth, int fHeight, int bpp) override;
  void CopyBuffer(uint32_t* mem) override;
  void Render() override;
  bool CheckEvents() override;

 private:
  SDL2VideoDisplay(int width, int height, const bool fullscreen);
  SDL_Window* window_;
  SDL_Surface* surface_;
  SDL_Surface* buffer_;
  int maxW_;
  int maxH_;
  int fWidth_;
  int fHeight_;
};

}  // namespace gvm

#endif  // _GVM_SDL2_VIDEO_DISPLAY_H_
