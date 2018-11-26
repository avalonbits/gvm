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
  void CopyBuffer(const uint32_t* mem) override;
  void Render() override;
  bool CheckEvents() override;

 private:
  void config();

  int maxW_;
  int maxH_;
  std::unique_ptr<sf::RenderWindow> window_;
  double w_scale_;
  double h_scale_;
  sf::Texture texture;
  std::unique_ptr<uint32_t> buffer_;
  uint32_t buffer_size_;
};

}  // namespce gvm

#endif  // _GVM_SFML_VIDEO_DISPLAY_H_
