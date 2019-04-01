#ifndef _GVM_INPUT_CONTROLLER_H_
#define _GVM_INPUT_CONTROLLER_H_

#include <functional>

namespace gvm {
class InputController {
 public:
  explicit InputController(std::function<void(uint32_t value)> callback);
  ~InputController();

  void Read();
  void Shutdown() { shutdown_ = true; }

 private:
  bool shutdown_;
  std::function<void(uint32_t value)> callback_;
};

}  // namepsace gvm

#endif  // _GVM_INPUT_CONTROLLER_H_
