#include "disk.h"

#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace gvm { 

FileBackedDisk::~FileBackedDisk() {
  if (map_ != nullptr) {
    close(file_descriptor_);
    map_ = nullptr;
  }
}

bool FileBackedDisk::Init() {
  if (map_ != nullptr) return true;

  const size_t file_size = Disk::kSectorSize * total_sectors_;
  file_descriptor_ = open(file_name_.c_str(), O_RDWR, (mode_t)0600);
  if (file_descriptor_ == -1) {
    // File doesn't exist. Create it with the correct size.
    file_descriptor_ = open(file_name_.c_str(), O_RDWR | O_CREAT, (mode_t)0600);
    if (file_descriptor_ == -1) {
      std::cerr << "Unable to open " << file_name_ << ".\n";
      return false;
    }

    auto result = lseek(file_descriptor_, file_size-1, SEEK_SET);
    if (result == -1) {
	  close(file_descriptor_);
      std::cerr << "Error calling lseek on " << file_name_ << "\n";
      return false;
    }

    // Write something to end of file to make sure the file has the size we need.
    result = write(file_descriptor_, "", 1);
    if (result == -1) {
      close(file_descriptor_);
      std::cerr << "Error writing to file " << file_name_ << " on init.\n";
      return false;
    }
  }

  // Finally we can mmap the file.
  map_ = reinterpret_cast<uint8_t*>(mmap(
      nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor_, 0));
  if (map_ == MAP_FAILED) {
    map_ = nullptr;
    close(file_descriptor_);
    std::cerr << "Unable to memory map " << file_name_ << "\n"; 
    return false;
  }

  // All is fine.
  return true;
}

int32_t FileBackedDisk::Read(
    uint32_t start_sector, uint32_t sector_count, uint8_t* mem) const {
  if (sector_count == 0) return 0;
  if (map_ == nullptr) return -1;
  if (start_sector >= total_sectors_) return -2;
  if (start_sector + sector_count > total_sectors_) {
    sector_count = total_sectors_ - start_sector;
  }
  const auto start_byte = start_sector * Disk::kSectorSize;
  const auto end_byte = (start_sector + sector_count) * Disk::kSectorSize;
  std::memcpy(mem, &map_[start_byte], end_byte - start_byte);

  return sector_count;
}

int32_t FileBackedDisk::Write(
    uint32_t start_sector, uint32_t sector_count, const uint8_t* mem) {
  if (sector_count == 0) return 0;
  if (map_ == nullptr) return -1;
  if (start_sector >= total_sectors_) return -2;
  if (start_sector + sector_count > total_sectors_) {
    sector_count = total_sectors_ - start_sector;
  }
  const auto start_byte = start_sector * Disk::kSectorSize;
  const auto end_byte = (start_sector + sector_count) * Disk::kSectorSize;
  std::memcpy(&map_[start_byte], mem, end_byte - start_byte);

  return sector_count;
}

}  // namespace gvm
