#ifndef _GVM_ROM_H_
#define _GVM_ROM_H_

#include <fstream>
#include <map>
#include <vector>

#include "isa.h"

namespace gvm {
class Rom {
 public:
  static const Rom* FromFile(std::ifstream& in);

  Rom(uint32_t memaddr, const std::vector<Word>& words);
  Rom(const Rom&) = delete;
  Rom& operator=(const Rom&) = delete;
  virtual ~Rom() {}

  void Load(uint32_t memaddr, const std::vector<Word>& words);
  void Load(uint32_t memaddr, const uint32_t* words, uint32_t size);
  const std::map<uint32_t, std::vector<Word>>& Contents() const { return rom_; }
  void ToFile(std::ofstream& out) const;

 private:
  Rom() {};
  std::map<uint32_t, std::vector<Word>> rom_;
};
}  // namespace gvm
#endif  // _GVM_ROM_H_
