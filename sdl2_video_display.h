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
  void CopyBuffer(uint32_t* mem) override;
  void Render() override;
  bool CheckEvents() override;

 private:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  int maxW_;
  int maxH_;
  int fWidth_;
  int fHeight_;
  int count_;
};

}  // namespace gvm

#endif  // _GVM_SDL2_VIDEO_DISPLAY_H_
