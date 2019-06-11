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
  volatile bool shutdown_;
  std::function<void(uint32_t value)> callback_;
};

}  // namepsace gvm

#endif  // _GVM_INPUT_CONTROLLER_H_
