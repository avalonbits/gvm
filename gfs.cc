#include "gfs.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace gvm {
namespace gfs {

void UCS2name(char* name16, const char* name8) {
  for (int i = 0; name8[i] != 0; ++i) {
    name16[2*i] = name8[i];
    name16[2*i+1] = 0;
  }
}

bool Partition(Disk* disk, PartitionTable* const table) {
  if (!disk->Init()) {
    std::cerr << "Disk was not properly initalized.\n";
    return false;
  }
  table->set_partitions &= 0xF;  // We only allow 4 partitions.

  // Enforce the magic number.
  table->magic = 0xEE1987EE;

  // Partitions will be written to sector 0.
  uint8_t block[Disk::kSectorSize];
  memset(block, 0, Disk::kSectorSize);
  std::memcpy(block, table, sizeof(PartitionTable));
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
  uint8_t block[Disk::kSectorSize];
  memset(&block[0], 0, Disk::kSectorSize);
  memcpy(&block[0], &sblock, sizeof(SuperBlock));
  auto sectors = disk->Write(part->start_sector, 1, block);
  if (sectors != 1) {
    std::cerr << "Unable to write the super block at  " << part->start_sector << ": "
              << sectors << "\n";
    return false;
  }

  // Now we need to write the bitmap to dislk.
  // First, calculate the number of used sectors for the bitmap
  int32_t used_sectors = 3;  // partition table, superblock and root inode sectors.
  used_sectors += std::ceil(
      sblock.bytes_for_bitmap / static_cast<double>(Disk::kSectorSize));
  // Set the used blocks in the bitmap.
  int32_t unmarked_sectors = used_sectors;
  for (uint32_t i = 0; i < Disk::kSectorSize; ++i) {
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
      memset(&block[0], 0, Disk::kSectorSize);
    }
  }

  // Finally, write the root inode.
  Inode root;
  memset(&root, 0, sizeof(Inode));
  memset(block, 0, Disk::kSectorSize);
  memcpy(block, &root, sizeof(Inode));
  sectors = disk->Write(part->start_sector + 1, 1, block);
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

  PartitionTable table;
  std::memcpy(&table, block, sizeof(PartitionTable));
  
  // Check is partition that will be formated is valid.
  const bool valid = (table.set_partitions >> partition) & 0x1;
  if (!valid) {
    std::cerr << "Partition " << partition << " is not valid.\n";
    return false;
  }

  return formatPartition(disk, &table.partitions[partition]);
}

// We only check the simple stuff: magic number and set_partions <= 0xF
bool IsDiskPartitioned(Disk* disk) {
  uint8_t block[Disk::kSectorSize];
  auto count = disk->Read(0, 1, block);
  if (count != 1) {
    std::cerr << "Error reading disk partion from sector 0.\n";
    return false;
  }

  PartitionTable table;
  memcpy(&table, block, sizeof(PartitionTable));
  return table.magic == 0xEE1987EE && table.set_partitions <= 0xF;
}

bool IsDiskPartitionFormated(Disk* disk, uint32_t partition) {
  uint8_t block[Disk::kSectorSize];
  auto count = disk->Read(0, 1, block);
  if (count != 1) {
    std::cerr << "Error reading disk partion from sector 0.\n";
    return false;
  }

  PartitionTable table;
  memcpy(&table, block, sizeof(PartitionTable));
  if (table.magic != 0xEE1987EE || table.set_partitions > 0xF) {
    std::cerr << "Disk is not partitioned.\n";
    return false;
  }

  if (!((table.set_partitions >> partition) & 0x01)) {
    std::cerr << "Partition " << partition << " is invalid.\n";
    return false;
  }

  auto* part = &table.partitions[partition];
  count = disk->Read(part->start_sector, 1, block);
  if (count != 1) {
    std::cerr << "Error reading partition " << partition << " super block from sector "
              << part->start_sector << ".\n";
    return false;
  }

  SuperBlock sb;
  memcpy(&sb, block, sizeof(SuperBlock));

  // Just check the magic number and hope for the best.
  return sb.magic == 0xFF1987FF;
}
}  // namespace gfs
}  // namespace gvm
