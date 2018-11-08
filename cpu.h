#ifndef _GVM_CPU_H_
#define _GVM_CPU_H_

#include <cstring>
#include <string>
#include <vector>

#include "isa.h"

namespace gvm {
constexpr uint32_t kTotalMem = 256 << 20;  // 256MiB
constexpr uint32_t kTotalWords = kTotalMem / gvm::kWordSize;

class CPU {
 public:
  CPU() : pc_(0) {
    std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
    std::memset(mem_, 0, kTotalWords * sizeof(uint32_t));
  }

  // Don't allow copy construction
  CPU(const CPU&) = delete;
  CPU& operator=(const CPU&) = delete;

  void LoadProgram(uint32_t start, const std::vector<Word>& program);
  void Run();
  void Run(uint32_t start);

  const std::string PrintRegisters();
  const std::string PrintMemory(uint32_t from, uint32_t to);

 private:
  uint32_t pc_;
  uint32_t reg_[kRegCount];
  uint32_t mem_[kTotalWords];
};

}  // namespace gvm

#endif
