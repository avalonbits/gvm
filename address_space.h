#ifndef _GVM_ADDRESS_SPACE_H_
#define _GVM_ADDRESS_SPACE_H_

#include <cstdint>
#include <functional>

#include "rom.h"

namespace gvm {

class AddressSpace {
 public:
  explicit AddressSpace(std::function<void(const uint32_t addr)> illegal_address);
  ~AddressSpace();

  bool LoadKernelRom(const Rom* rom);
  void ConnectVideoSignal(
      std::function<void(uint32_t* iovalue,
                         uint32_t* vram,
                         const uint32_t size)> video_signal);

  uint32_t& operator[](std::size_t idx) const;

 private:
  uint32_t* mem_;
  std::function<void(const uint32_t addr)> illegal_address_;
  std::function<void(uint32_t* iovalue,
                     uint32_t* vram,
                     const uint32_t size)> video_signal_;
};

}  // namespace gvm

#endif  // _GVM_ADDRESS_SPACE_H_
