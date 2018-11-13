#ifndef _GVM_ROM_H_
#define _GVM_ROM_H_

#include <vector>

#include "isa.h"

namespace gvm {
class Rom {
 public:
  explicit Rom(uint32_t size);
  Rom(const Rom&) = delete;
  Rom& operator=(const Rom&) = delete;

  bool Set(uint32_t memaddr, Word word);
  const std::vector<Word>& Contents() { return rom_; }

 private:
  std::vector<Word> rom_;
};
}  // namespace gvm
#endif  // _GVM_ROM_H_
