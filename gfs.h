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

bool Partition(Disk* disk, DiskPartition partitions[4]);
bool Format(Disk* disk, uint32_t partition);

}  // namespace gfs
}  // namespace gvm

#endif  // _GVM_GFS_H_

