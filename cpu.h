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

  uint32_t PowerOn();
  uint32_t Reset();

  const std::string PrintRegisters(bool hex = false);
  const std::string PrintMemory(uint32_t from, uint32_t to);
  const std::string PrintStatusFlags();

 private:
  void Run();
  void SetPC(uint32_t pc);

  std::string PrintInstruction(const Word word);

  uint32_t& pc_;
  uint32_t* mem_;
  uint32_t reg_[kRegCount];
  uint32_t mem_size_;
  uint32_t& sp_;
  uint32_t& fp_;
  uint32_t interrupt_;
  uint32_t op_count_;
};

}  // namespace gvm

#endif  // _GVM_CPU_H_
