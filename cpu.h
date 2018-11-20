#ifndef _GVM_CPU_H_
#define _GVM_CPU_H_

#include <cstring>
#include <string>
#include <vector>

#include "isa.h"
#include "video_controller.h"

namespace gvm {
class CPU {
 public:
  CPU();

  // Don't allow copy construction
  CPU(const CPU&) = delete;
  CPU& operator=(const CPU&) = delete;

  void ConnectMemory(uint32_t* mem, uint32_t mem_size_bytes);
  void RegisterVideoDMA(VideoController* controller);
  void LoadProgram(uint32_t start, const std::vector<Word>& program);
  void SetPC(uint32_t pc);
  const bool Step();
  uint32_t Run(const bool debug);

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  uint32_t pc_;
  uint32_t reg_[kRegCount];
  uint32_t* mem_;
  uint32_t mem_size_;
  uint32_t& sp_;
  uint32_t& fp_;
  uint8_t sflags_;
};

}  // namespace gvm

#endif  // _GVM_CPU_H_
