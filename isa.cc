#include "isa.h"

#include <cassert>

namespace gvm {

Word Nop() {
  return Word(ISA::NOP);
}

Word Halt() {
  return Word(ISA::HALT);
}

Word LoadRR(uint32_t dest, uint32_t src) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(dest != src);
  return Word(ISA::LOAD_RR | dest << 8 | src << 12);
}

Word LoadRI(uint32_t dest, uint32_t value) {
  assert(dest < kRegCount);
  assert(value == (value & 0xFFFFF));  // mask 20 bits.
  return Word(ISA::LOAD_RI |dest << 8 | value << 12);
}

Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::ADD_RR | dest << 8 | op1 << 12 | op2 << 16);
}

}  // namespace gvm
