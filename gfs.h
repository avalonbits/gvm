#ifndef _GVM_GFS_H_
#define _GVM_GFS_H_

#include "disk.h"
namespace gvm {
namespace gfs {

bool Partition(Disk* disk);
bool Format(Disk* disk);

}  // namespace gfs
}  // namespace gvm

#endif  // _GVM_GFS_H_

