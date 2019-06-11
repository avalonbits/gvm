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

#ifndef _GVM_ROM_H_
#define _GVM_ROM_H_

#include <fstream>
#include <map>
#include <vector>

#include "isa.h"

namespace gvm {
class Rom {
 public:
  static const Rom* FromFile(std::ifstream& in);

  Rom(uint32_t memaddr, const std::vector<Word>& words);
  Rom(const Rom&) = delete;
  Rom& operator=(const Rom&) = delete;
  virtual ~Rom() {}

  void Load(uint32_t memaddr, const std::vector<Word>& words);
  void Load(uint32_t memaddr, const uint32_t* words, uint32_t size);
  const std::map<uint32_t, std::vector<Word>>& Contents() const { return rom_; }
  void ToFile(std::ofstream& out) const;

  const uint32_t Size() const;

 private:
  Rom() {};
  std::map<uint32_t, std::vector<Word>> rom_;
};
}  // namespace gvm
#endif  // _GVM_ROM_H_
