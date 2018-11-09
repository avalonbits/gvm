#include "isa.h"

#include <cassert>
#include <iostream>

#include "cpu.h"

namespace gvm {

Word Nop() {
  return Word(ISA::NOP);
}

Word Halt() {
  return Word(ISA::HALT);
}

Word MovRR(uint32_t dest, uint32_t src) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(dest != src);
  return Word(ISA::MOV_RR | dest << 8 | src << 12);
}

Word MovRI(uint32_t dest, uint32_t value) {
  assert(dest < kRegCount);
  value = value & 0xFFFFF;
  return Word(ISA::MOV_RI| dest << 8 | value << 12);
}

Word LoadRI(uint32_t dest, uint32_t memaddr) {
  assert(dest < kRegCount);
  assert((memaddr & 0xFFFFF) == memaddr);
  assert(memaddr % kWordSize == 0);  // kWordSize aligned memory.
  return Word(ISA::LOAD_RI | dest << 8 | memaddr << 12);
}

Word LoadRR(uint32_t dest, uint32_t src) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  return Word(ISA::LOAD_RR | dest << 8 | src << 12);
}

Word StorRI(uint32_t memaddr, uint32_t src) {
  assert((memaddr & 0xFFFFF) == memaddr);
  assert(memaddr % kWordSize == 0);  // kWordSize aligned memory.
  assert(src < kRegCount);
  return Word(ISA::STOR_RI | src << 8 | memaddr << 12);
}

Word StorRR(uint32_t dest, uint32_t src) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  return Word(ISA::STOR_RR | dest << 8 | src << 12);
}

Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::ADD_RR | dest << 8 | op1 << 12 | op2 << 16);
}

Word SubRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::SUB_RR | dest << 8 | op1 << 12 | op2 << 16);
}

Word Jmp(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JMP | memaddr << 8);
}

Word Jne(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JNE | memaddr << 8);
}

Word Jeq(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JEQ | memaddr << 8);
}

Word Jgt(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JGT | memaddr << 8);
}

Word Jge(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JGE | memaddr << 8);
}

Word Jlt(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JLT | memaddr << 8);
}

Word Jle(uint32_t memaddr) {
  memaddr = memaddr & 0xFFFFFF;
  return Word(ISA::JLE | memaddr << 8);
}

}  // namespace gvm
