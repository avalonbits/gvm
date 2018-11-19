#include "rom.h"

#include <cassert>

namespace gvm {

Rom::Rom(const std::vector<Word>& words) {
  assert(!words.empty());
  rom_ = words;
}

Rom::Rom(const Word* words, uint32_t size) {
  assert(size > 0);
  assert(words != nullptr);
  rom_.reserve(size);
  for (uint32_t i = 0; i < size; ++i) {
    rom_.push_back(words[i]);
  }
}

bool Rom::Set(uint32_t memaddr, Word word) {
  if (memaddr % kWordSize != 0) return false;

  const uint32_t pos = memaddr / kWordSize;
  if (pos > rom_.size()) return false;

  rom_[pos] = word;
  return true;
}

}  // namespace gvm
