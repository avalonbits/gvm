#ifndef _GVM_GFS_H_
#define _GVM_GFS_H_

#include "disk.h"
namespace gvm {
namespace gfs {

typedef struct {
  uint32_t start_sector;
  uint32_t end_sector;
  char label[24];
} DiskPartition;

typedef struct {
  uint32_t magic;
  DiskPartition partitions[4];
  uint32_t set_partitions;
} PartitionTable;

void UCS2name(char* name16, const char* name8); 
bool Partition(Disk* disk, PartitionTable* const table);
bool Format(Disk* disk, uint32_t partition);
bool IsDiskPartitioned(Disk* disk);
bool IsDiskPartitionFormated(Disk* disk, uint32_t partition);

}  // namespace gfs
}  // namespace gvm

#endif  // _GVM_GFS_H_

