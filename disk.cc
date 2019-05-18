#include "disk.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace gvm { 

FileBackedDisk::~FileBackedDisk() {
  if (map_ != nullptr) {
    close(file_descriptor_);
  }
}

bool FileBackedDisk::Init() {
  return false; 
}

int32_t FileBackedDisk::Read(
    uint32_t start_sector, uint32_t sector_count, uint32_t* mem) const {
  return -1;
}

int32_t FileBackedDisk::Write(
    uint32_t start_sector, uint32_t sector_count, const uint32_t* mem) {
  return -1;
}


}  // namespace gvm
