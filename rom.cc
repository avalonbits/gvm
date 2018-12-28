#include "rom.h"

#include <cassert>

namespace gvm {

Rom::Rom(uint32_t memaddr, const std::vector<Word>& words) {
  Load(memaddr, words);
}

void Rom::Load(uint32_t memaddr, const std::vector<Word>&  words) {
  assert(!words.empty());
  assert(memaddr % kWordSize == 0);
  rom_[memaddr] = words;
}

}  // namespace gvm
