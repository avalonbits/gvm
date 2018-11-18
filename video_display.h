#ifndef _GVM_VIDEO_DISPLAY_H_
#define _GVM_VIDEO_DISPLAY_H_

#include <atomic>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gvm {

class VideoDisplay {
 public:
  VideoDisplay();  // Fullscreen display
  VideoDisplay(int x, int y);  // Windowed display.
  ~VideoDisplay();

  void SendData(uint32_t* data, uint32_t byte_count);
  void RenderLoop();
  void Shutdown() {
    shutdown_ = true;
  }
  uint32_t RequiredMemoryInBytes() const {
    return sizeof(uint32_t) * (maxX_ * maxY_) / text_scale_;
  }

 private:
  void config();

  int maxX_;
  int maxY_;
  std::unique_ptr<sf::RenderWindow> window_;
  double text_scale_;

  uint32_t mem_size_;
  uint32_t* mem_;
  uint32_t* buffer_;
  std::atomic<bool> new_buffer_;
  std::atomic<bool> shutdown_;
};

}  // namespace gvm

#endif  // _GVM_VIDEO_DISPLAY_H_
