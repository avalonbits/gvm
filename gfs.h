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

