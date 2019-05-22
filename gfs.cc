#include "gfs.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace gvm {
namespace gfs {

typedef struct {
  uint32_t start_sector;
  uint32_t end_sector;
  char label[24];
} DiskPartition;

bool Partition(Disk* disk, DiskPartition partitions[4]) {
  if (!disk->Init()) {
    std::cerr << "Disk was not properly initalized.\n";
    return false;
  }

  for (int i = 0; i < 4; ++i) {
    // Check that partitions are valid.
  }

  // Partitions will be written to sector 0.
  uint8_t block[Disk::kSectorSize];
  std::memcpy(block, &partitions, sizeof(DiskPartition)*4);
  auto bytes = disk->Write(0, 1, &block[0]);
  if (bytes < 0) {
    std::cerr << "Error writing partition information to block 0 of disk: "
              << bytes << "\n";
    return false;
  }
  if (bytes != Disk::kSectorSize) {
    std::cerr << "Wrote less then 1 block: "  << bytes << "\n";
    return false;
  }
  return true;
}

typedef struct {
  uint32_t magic;
  uint32_t total_sectors;
  uint32_t total_inodes;

  // The number of bytes need to keep track of free sectors.
  // if it is <= 496 then bitmap is contained within the same sector of the
  // super block. Otherwise we allocate one extra sector for every 512 bytes.
  // This value will be ceil(total_sectors / 8.0);
  uint32_t bytes_bitmap_count;
  uint8_t sec_bitmap[496];
} SuperBlock;


static bool formatPartition(Disk* disk, DiskPartition* const part) {
  if (part->start_sector >= part->end_sector) {
    std::cerr << "Invalid partion: " << part->start_sector << " >= " << part->end_sector
              << std::endl;
    return false;
  }

  // GFS Partitions must be at least 1MiB in size. That means 2048 sectors.
  const auto total_sectors = part->end_sector - part->start_sector;
  if (total_sectors < 2048) {
    std::cerr << "GFS partitions must be at least 1MiB in size (2048 sectors): "
              << total_sectors << "\n";
    return false;
  }

  // The number of sectors reserverd for inodes is determined as follows:
  //   2048 - 10239 sectors: 200 sectors (800 inodes)
  //  10240 - Max sectors: 2%
  SuperBlock sblock;
  sblock.magic = 0xFF1987FF;
  sblock.total_sectors = total_sectors;
  sblock.total_inodes = total_sectors < 10240
    ? 200
    : static_cast<uint32_t>(std::ceil(total_sectors * 0.02));
  sblock.bytes_bitmap_count = static_cast<uint32_t>(std::ceil(total_sectors / 8.0));

  // Calculate the number of extra sectors for the bitmap
  int32_t used_sectors = 2;  // partition table and superblock each use a sector.
  if (sblock.bytes_bitmap_count > 496) {
    used_sectors += std::ceil((sblock.bytes_bitmap_count - 496) / 512.0);
  }
  std::memset(&sblock.sec_bitmap[0], 0, 496);

  // Set the used blocks in the bitmap.
  // TODO(icc): Support disks that require more than 496 bytes for free secotr bitmap.
  for (int i = 0; i < 496; ++i) {
    used_sectors -= 8;
    if (used_sectors >= 0) {
      sblock.sec_bitmap[i] = 0xFF;
      if (used_sectors == 0) break;
    } else {
      sblock.sec_bitmap[i] = 1;
      used_sectors += 8;
      while (--used_sectors > 0) {
        sblock.sec_bitmap[i] <<= 1;
        sblock.sec_bitmap[i] |= 1;
      }
      break;
    }
  }

  // Super block is the first sector of the partition.
  auto bytes = disk->Write(part->start_sector, 1, reinterpret_cast<uint8_t*>(&sblock));
  if (bytes < 0 || bytes < static_cast<int32_t>(Disk::kSectorSize)) {
    std::cerr << "Unable to write the super block at  " << part->start_sector << ": "
              << bytes << "\n";
    return false;
  }


  // TODO(icc): write the root inode.
  return true;
}

bool Format(Disk* disk, uint32_t partition) {
  if (!disk->Init()) {
    std::cerr << "Disk was not properly initalized.\n";
    return false;
  }
  if (partition > 3) {
    std::cerr << "Only four partitions are recognizable.\n";
    return false;
  }

  // Read the partition table and check if the partition is valid.
  uint8_t block[Disk::kSectorSize];
  auto bytes = disk->Read(0, 1, &block[0]);
  if (bytes < 0) {
    std::cerr << "Error reading partition information from block 0 of disk: "
              << bytes << "\n";
    return false;
  }

  DiskPartition partitions[4];
  std::memcpy(partitions, block, sizeof(DiskPartition) * 4);

  return formatPartition(disk, &partitions[partition]);
}

}  // namespace gfs
}  // namespace gvm
