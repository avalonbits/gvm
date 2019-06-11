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

#include "rom.h"

#include <cassert>

namespace gvm {

/* static */
const Rom* Rom::FromFile(std::ifstream& in) {
  const std::string header("s1987gvm");

  char hBuffer[8];
  in.read(hBuffer, 8);
  assert(header.compare(0, 8, hBuffer, 8) == 0);

  auto* rom = new Rom();
  while (in) {
    uint32_t addr;
    in.read(reinterpret_cast<char*>(&addr), 4);
    if (!in) break;
    uint32_t wCount;
    in.read(reinterpret_cast<char*>(&wCount), 4);
    assert(wCount <= (256 << 20));
    uint32_t* buffer = new uint32_t[wCount];
    in.read(reinterpret_cast<char*>(buffer), wCount*4);
    rom->Load(addr, buffer, wCount);
  }
  in.close();
  return rom;
}

Rom::Rom(uint32_t memaddr, const std::vector<Word>& words) {
  Load(memaddr, words);
}

void Rom::Load(uint32_t memaddr, const std::vector<Word>&  words) {
  assert(!words.empty());
  assert(memaddr % kWordSize == 0);
  rom_[memaddr] = words;
}

void Rom::Load(uint32_t memaddr, const uint32_t* words, uint32_t size) {
  std::vector<Word>  program(words, words + size);
  Load(memaddr, program);
}

void Rom::ToFile(std::ofstream& out) const {
  std::vector<uint32_t> buffer;

  for (const auto& kv : rom_) {
    buffer.push_back(kv.first);
    buffer.push_back(kv.second.size());
    buffer.insert(buffer.end(), kv.second.begin(), kv.second.end());
  }

  out.write("1987gvm", 7);
  out.write(reinterpret_cast<const char*>(buffer.data()),
            buffer.size() * sizeof(uint32_t));
  out.close();
}

const uint32_t Rom::Size() const {
  uint32_t size = 0;
  for (const auto kv : rom_) {
    size += kv.second.size();
  }
  return size * sizeof(Word);
}

}  // namespace gvm
