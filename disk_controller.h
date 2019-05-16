#ifndef _GVM_DISK_CONTROLLER_H_
#define _GVM_DISK_CONTROLLER_H_

#include <cassert>
#include <cstdint>

#include "sync_types.h"

namespace gvm {

/**
 * Supported ops:
 * READ: disk, dmaid, first sector, sector count.
 * WRITE: disk, dmaid, first sector, sector count.
 * DMA: disk, dmaid, buffer sector count, start address as multiple of sector size.
 * STAT: no params, should return disk count and dmaids associated, if available.
 *
 * Max disks supported: 2 (2 bit)
 * Max dma buffers per disk: 4 (2 bits)
 * Sector size: 512 bytes (2^9)
 * Max disk size: 1 GiB (2^30 bytes)
 * Max partition size: 256 MiB (2^26 bytes)
 * Max sector count per partition: 2^17 (128k sectors)
 *
 * READ/WRITE Commands are all issued using a 32 bit word.
 * 0-2 bit: command
 * 3: disk (0 or 1)
 * 4-5: dmaid (0-3)
 * 6-22: first sector (0 - 2^17)
 * 21-32: sector count (0 - 2^10 sectors [1MiB])
 *
 * DMA is similar but the address is assumed to be aligned with sector size. 
 * 0-2 bits: command
 * 3: disk (0 or 1)
 * 4-5: dmaid (0-3)
 * 6-7: buffer size (128k - 1MiB)
 * 8-32: address to write to (Limited to first 32MiB)
 *
 * STAT is actually multi purpose:
 * 0-2: command
 * 3-5: sub commands:
 *       - 0: return disk count
 *       - 1: return dmaid list for disk
 *       - 2: return config of dmaid
 * 6-32: params for each subcommand.
 */
class DiskController {
 public:
  DiskController(SyncChan<uint32_t>* chan, uint32_t* mem)
    : chan_(chan), mem_(mem) {
    assert(chan_ != nullptr);
    assert(mem_ != nullptr);
  }

  void Start();
  void Stop() {
    chan_->send(0);
    chan_->Close();
  }

 private:
  SyncChan<uint32_t>* chan_;
  uint32_t* mem_;
};

}  // namespace gvm

#endif  // _GVM_DISK_CONTROLLER_H_
