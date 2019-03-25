#include "address_space.h"

#include <cassert>

namespace gvm {

namespace {
  uint32_t kDummy = 0xFFFFFFFF;

  constexpr const uint32_t m2w(const uint32_t addr) {
    return addr / sizeof(uint32_t);
  }
  const uint32_t kKernelMemSize = 1024 * 1024;
  const uint32_t kVramSize = 640 * 360 * 4;
  const uint32_t kUnmappedVram = 1024 * 1024 - kVramSize;
  const uint32_t kUserMemSize = 15 * 1024 * 1024 + kUnmappedVram;
  const uint32_t kUnicodeBitmapFont = 1024 * 1024;
  const uint32_t kIOMemSize = 8;
  const uint32_t kMemLimit =
      kKernelMemSize + kUserMemSize + kVramSize + kUnicodeBitmapFont + kIOMemSize;
  const uint32_t kVramStart = kKernelMemSize + kUserMemSize;
  const uint32_t kUnicodeRomStart = kVramStart + kVramSize;
  const uint32_t kIOStart = kUnicodeRomStart + kUnicodeBitmapFont;
  const uint32_t kVramReg = kIOStart;
  const uint32_t kInputReg = kIOStart + 4;
}  // namespace

AddressSpace::AddressSpace(std::function<void(const uint32_t addr)> illegal_address)
  : mem_(new uint32_t[m2w(kMemLimit)]), illegal_address_(illegal_address) {
}

AddressSpace::~AddressSpace() {
  delete [] mem_;
}

uint32_t& AddressSpace::operator[](std::size_t idx) const {
  if (idx >= kMemLimit || idx % sizeof(uint32_t) != 0) {
    illegal_address_(idx);
    return kDummy;
  }

  if (idx == kVramReg) {
    video_signal_(&mem_[m2w(kVramReg)], &mem_[m2w(kVramStart)], kVramSize);
  } else if (idx == kInputReg) {
    illegal_address_(idx);
    return kDummy;
  }

  return mem_[m2w(idx)];
}

bool AddressSpace::LoadKernelRom(const Rom* rom) {
  if (rom->Size() > kKernelMemSize) {
    return false;
  }

  const auto& program = rom->Contents();
  for (const auto& kv : program) {
    const auto& start = m2w(kv.first);
    const auto& words = kv.second;
    assert(!words.empty());
    assert(words.size() + start < m2w(kKernelMemSize));
    for (uint32_t idx = start, i = 0; i < words.size(); ++idx, ++i) {
      mem_[idx] = words[i];
    }
  }
  return true;
}

}  // namespace gvm
