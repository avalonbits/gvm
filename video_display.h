#ifndef _GVM_VIDEO_DISPLAY_H_
#define _GVM_VIDEO_DISPLAY_H_

#include <atomic>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gvm {

class VideoDisplay {
 public:
  VideoDisplay();  // Fullscreen display
  VideoDisplay(int width, int height);  // Windowed display.
  ~VideoDisplay();

  void SetFramebufferSize(int fWidth, int fHeight, int bpp);
  void Render(const uint32_t* mem);
  bool CheckEvents();

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

}  // namespace gvm

#endif  // _GVM_VIDEO_DISPLAY_H_
