#include "rom.h"

#include <cassert>

namespace gvm {

Rom::Rom(uint32_t size) {
  assert(size > 0);
  rom_.resize(size, 0);
}
bool Rom::Set(uint32_t memaddr, Word word) {
  if (memaddr % kWordSize != 0) return false;

  const uint32_t pos = memaddr / kWordSize;
  if (pos > rom_.size()) return false;

  rom_[pos] = word;
  return true;
}

}  // namespace gvm
