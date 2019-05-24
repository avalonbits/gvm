#ifndef _GVM_DISK_CONTROLLER_H_
#define _GVM_DISK_CONTROLLER_H_

#include <cassert>
#include <cstdint>
#include <vector>

#include "disk.h"
#include "sync_types.h"

namespace gvm {

/**
 * Supported ops:
 * READ: disk, dmaid, first sector, sector count.
 * WRITE: disk, dmaid, first sector, sector count.
 * DMA: disk, dmaid, buffer sector count, start address as multiple of sector size.
 * STAT: no params, should return disk count and dmaids associated, if available.
 *
 * Max disks supported: 2 (1 bit)
 * Max dma buffers per disk: 4 (2 bits)
 * Sector size: 512 bytes (2^9)
 * Max disk size: 1 GiB (2^30 bytes)
 * Max partition size: 256 MiB (2^28 bytes)
 * Max sector count per partition: 2^19 (512k sectors)
 *
 * READ/WRITE Commands are all issued using a 32 bit word.
 * 0-2 bit: command
 * 3: disk (0 or 1)
 * 4-5: dmaid (0-3)
 * 6-24: first sector (0 - 2^19))
 * 25-32: sector count (0 - 2^8 sectors [128KiB])
 *
 * DMA is similar. It assumes 128k buffer sizes and address is limited to 27 bits.
 * 0-2 bits: command
 * 3: disk (0 or 1)
 * 4-5: dmaid (0-3)
 * 6-32: address to write to (limited to first 128MiB))
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
  explicit DiskController(const std::vector<Disk*>& disks)
    : disks_(disks) {
  }

  void SetMemRegister(SyncChan<uint32_t>* chan) {
    chan_ = chan;
  }

  void Start();
  void Stop() {
    chan_->send(0);
    chan_->Close();
  }

 private:
  SyncChan<uint32_t>* chan_;
  std::vector<Disk*> disks_;
  uint32_t* mem_;
};

}  // namespace gvm

#endif  // _GVM_DISK_CONTROLLER_H_
