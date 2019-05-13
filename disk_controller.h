#ifndef _GVM_DISK_CONTROLLER_H_
#define _GVM_DISK_CONTROLLER_H_

#include <cassert>
#include <cstdint>

#include "sync_types.h"

namespace gvm {

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
