/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
