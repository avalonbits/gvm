#ifndef _GVM_CPU_H_
#define _GVM_CPU_H_

#include <cstring>
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

        void LoadProgram(const std::vector<Word>& program);
        void Run();

    private:
        uint32_t pc_;
        uint32_t reg_[kRegCount];
        uint32_t mem_[kTotalWords];

};

}  // namespace gvm

#endif
