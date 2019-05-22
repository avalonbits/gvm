#include "gfs.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace gvm {
namespace gfs {

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
  std::memcpy(block, partitions, sizeof(DiskPartition)*4);
  auto sectors = disk->Write(0, 1, &block[0]);
  if (sectors != 1) {
    std::cerr << "Error writing partition information to sector 0 of disk: "
              << sectors << "\n";
    return false;
  }
  return disk->Fsync();
}

typedef struct {
  uint32_t magic;
  uint32_t total_sectors;
  uint32_t total_inodes;

  // The number of bytes need to keep track of free sectors.
  uint32_t bytes_for_bitmap;
} SuperBlock;

typedef struct {
  char name[64];
  uint32_t attr;
  uint32_t size;
  uint32_t direct[11];
  uint32_t indirect;
  uint32_t d_indirect;
  uint32_t t_inderict;
} Inode;

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
  sblock.bytes_for_bitmap = static_cast<uint32_t>(std::ceil(total_sectors / 8.0));

  // Super block is the first sector of the partition.
  auto sectors = disk->Write(part->start_sector, 1, reinterpret_cast<uint8_t*>(&sblock));
  if (sectors != 1) {
    std::cerr << "Unable to write the super block at  " << part->start_sector << ": "
              << sectors << "\n";
    return false;
  }

  // Now we need to write the bitmap to dislk.
  // First, calculate the number of used sectors for the bitmap
  int32_t used_sectors = 3;  // partition table, superblock and root inode sectors.
  used_sectors += std::ceil(sblock.bytes_for_bitmap / 512.0);

  uint8_t block[512];
  memset(&block[0], 0, 512);

  // Set the used blocks in the bitmap.
  // TODO(icc): Support disks that require more than 512 bytes for used sector bitmap.
  int32_t unmarked_sectors = used_sectors;
  for (int i = 0; i < 512; ++i) {
    unmarked_sectors -= 8;
    if (unmarked_sectors >= 0) {
      block[i] = 0xFF;
      if (unmarked_sectors == 0) break;
      continue;
    }

    // At this point we are at the last byte of the bitmap.
    block[i] = 1;
    unmarked_sectors += 8;
    while (--unmarked_sectors > 0) {
      block[i] <<= 1;
      block[i] |= 1;
    }
    break;
  }

  // Write the bitmap blocks. It starts at start_sector + 2 because inode
  // 0 is always at start_sect + 1.
  for (int i = 0; i < used_sectors; ++i) {
    const auto sec = part->start_sector + 2 + i;
    auto count = disk->Write(sec, 1, &block[0]);
    if (count != 1) {
      std::cerr << "Unable to write bitmap entry at sector " << sec << "\n";
      return false;
    }
    if (i == 0) {
      memset(&block[0], 0, 512);
    }
  }

  // Finally, write the root inode.
  Inode root;
  memset(&root, 0, sizeof(Inode));
  sectors = disk->Write(part->start_sector + 1, 1, reinterpret_cast<uint8_t*>(&root));
  if (sectors != 1) {
    std::cerr << "Unable to write root inode: " << sectors << std::endl;
    return false;
  }
  return disk->Fsync();
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
  auto sectors = disk->Read(0, 1, &block[0]);
  if (sectors != 1) {
    std::cerr << "Error reading partition information from block 0 of disk: "
              << sectors << "\n";
    return false;
  }

  DiskPartition partitions[4];
  std::memcpy(partitions, block, sizeof(DiskPartition) * 4);

  return formatPartition(disk, &partitions[partition]);
}

}  // namespace gfs
}  // namespace gvm
