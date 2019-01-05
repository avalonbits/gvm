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
  CPU(const uint32_t freq, const uint32_t fps);

  // Don't allow copy construction
  CPU(const CPU&) = delete;
  CPU& operator=(const CPU&) = delete;

  void ConnectMemory(uint32_t* mem, uint32_t mem_size_bytes);
  void RegisterVideoDMA(VideoController* controller);
  void LoadProgram(const std::map<uint32_t, std::vector<Word>>& program);
  void SetPC(uint32_t pc);
  const bool Step(const bool debug);
  uint32_t Run(const bool debug);

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  std::string PrintInstruction(const Word word);

  uint32_t& pc_;
  uint32_t* mem_;
  uint32_t reg_[kRegCount];
  uint32_t mem_size_;
  uint32_t& sp_;
  uint32_t& fp_;
  const uint32_t cycles_per_frame_;
  const uint32_t fps_;
};

}  // namespace gvm

#endif  // _GVM_CPU_H_
