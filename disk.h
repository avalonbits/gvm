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

#ifndef _GVM_DISK_H_
#define _GVM_DISK_H_

#include <cstdint>
#include <string>

namespace gvm {
class Disk {
 public:
  constexpr static uint32_t kSectorSize = 512 /* bytes */;

  Disk() {}
  virtual ~Disk() {}

  // If init is successful, it should not fail if called multiple times.
  virtual bool Init() = 0;

  virtual int32_t Read(
      uint32_t start_sector, uint32_t sector_count, uint8_t* mem) const = 0;
  virtual int32_t Write(
      uint32_t start_sector, uint32_t sector_count, const uint8_t* mem) = 0;
  virtual uint32_t Sectors() const = 0;
  virtual bool Fsync() = 0;
};


class FileBackedDisk : public Disk {
 public:
  FileBackedDisk(const std::string& file_name, uint32_t total_sectors)
    : file_name_(file_name), total_sectors_(total_sectors), map_(nullptr) {}
  ~FileBackedDisk() override;

  bool Init() override;

  int32_t Read(
      uint32_t start_sector, uint32_t sector_count, uint8_t* mem) const override;
  int32_t Write(
      uint32_t start_sector, uint32_t sector_count, const uint8_t* mem) override;
  uint32_t Sectors() const override {
    if (map_ == nullptr) return 0;
    return total_sectors_;
  }

  bool Fsync() override;

 private:
  std::string file_name_;
  const uint32_t total_sectors_;
  uint8_t* map_;
  int file_descriptor_;
};

}  // namespace gvm

#endif  // _GVM_DISK_H_
