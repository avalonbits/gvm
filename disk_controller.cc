#include "disk_controller.h"

namespace gvm {

void DiskController::Start() {
  while (true) {
    auto value = chan_->recv();
    const auto cmd = value & 0xF;
    if (cmd <= 0) break;
  }
}

}  // namespace gvm
