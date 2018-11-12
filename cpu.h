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
  CPU() : pc_(0), sp_(reg_[14]), fp_(reg_[15]), sflags_(0) {
    std::memset(reg_, 0, kRegCount * sizeof(uint32_t));
    std::memset(mem_, 0, kTotalWords * sizeof(uint32_t));
    fp_ = sp_ = kTotalWords;
  }

  // Don't allow copy construction
  CPU(const CPU&) = delete;
  CPU& operator=(const CPU&) = delete;

  void LoadProgram(uint32_t start, const std::vector<Word>& program);
  void SetPC(uint32_t pc);
  const bool Step();
  uint32_t Run() {
    uint32_t i = 1;
    while(Step()) { ++i; }
    return i;
  }

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  uint32_t pc_;
  uint32_t reg_[kRegCount];
  uint32_t mem_[kTotalWords];
  uint32_t& sp_;
  uint32_t& fp_;
  uint8_t sflags_;
};

}  // namespace gvm

#endif  // _GVM_CPU_H_
