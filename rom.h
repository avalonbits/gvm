#ifndef _GVM_ROM_H_
#define _GVM_ROM_H_

#include <vector>
#include <map>

#include "isa.h"

namespace gvm {
class Rom {
 public:
  Rom(uint32_t memaddr, const std::vector<Word>& words);
  Rom(const Rom&) = delete;
  Rom& operator=(const Rom&) = delete;
  virtual ~Rom() {}

  void Load(uint32_t memaddr, const std::vector<Word>& words);
  const std::map<uint32_t, std::vector<Word>>& Contents() const { return rom_; }

 private:
  std::map<uint32_t, std::vector<Word>> rom_;
};
}  // namespace gvm
#endif  // _GVM_ROM_H_
