#ifndef _GVM_SFML_VIDEO_DISPLAY_H_
#define _GVM_SFML_VIDEO_DISPLAY_H_

#include "video_display.h"

namespace gvm {

class SFMLVideoDisplay : public VideoDisplay {
 public:
  SFMLVideoDisplay();  // Fullscreen display
  SFMLVideoDisplay(int width, int height);  // Windowed display.
  ~SFMLVideoDisplay() override;

  void SetFramebufferSize(int fWidth, int fHeight, int bpp) override;
  void CopyBuffer(uint32_t* mem) override;
  void Render() override;
  bool CheckEvents() override;

 private:
  void config();
  void UpdateWindowSize(int wWidth, int wHeight);

  int maxW_;
  int maxH_;
  std::unique_ptr<sf::RenderWindow> window_;
  double w_scale_;
  double h_scale_;
  int fWidth_;
  int fHeight_;
  sf::Texture texture_;
  uint32_t buffer_size_;
};

}  // namespce gvm

#endif  // _GVM_SFML_VIDEO_DISPLAY_H_
