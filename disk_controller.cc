#include "disk_controller.h"

namespace gvm {

void DiskController::Start() {
  while (true) {
    auto value = chan_->recv();
    if (value == 0) {
      // value == 0 would mean read from disk 0, dma 0, sector 0, 0 sectors. We use
      // that to signal that the controller should be turned off.
      break;
    }
    const auto cmd = value & 0x3;  // command is first 2 bits
    switch (cmd) {
      case 0: /* READ */ {
        break;
      }
      case 1: /* WRITE */ {
        break;
      }
      case 2: /* DMA */ {
        break;
      }
      case 3: /* STAT */ {
        break;
      }
      default:
        break;
    }
  }
}

}  // namespace gvm
