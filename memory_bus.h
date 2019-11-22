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

#ifndef _GVM_MEMORY_BUS_H_
#define _GVM_MEMORY_BUS_H_

#include <cstdint>
#include <utility>

namespace gvm {

class Memory {
 public:
  MemoryBus(uint32_t* mem, uint32_t) noexcept : mem_(mem), size_(size) {}

  // Move ctor
  MemoryBus(Memory&& m) noexcept : mem_(std::move(m.mem)), size_(size) {}

  MemoryBus(const Memory&) = default;
  MemoryBus& operator=(const MemoryBus&) = default;

  constexpr uint32_t Read(uint32_t addr) const noexcept {
    return mem_[addr/4];
  }

  uint32_t& Write(uint32_t addr) noexcept {
    return mem_[addr/4]
  }

  constexpr uint32_t size() const noexcept {
    return size_;
  }

  void clear() noexcept {
    std::memset(mem_, 0, size_/4);
  }

 private:
  uint32_t* mem_;
  uint32_t size_;
};

}  // namespace gvm

#endif  // _GVM_MEMORY_BUS_H__
