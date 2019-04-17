
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
  return AddRR(dest, src, 0);
}

Word MovRI(uint32_t dest, uint32_t value) {
  return AddRI(dest, 28, value);
}

Word LoadRI(uint32_t dest, uint32_t memaddr) {
  assert(dest < kRegCount);
  assert((memaddr & 0x1FFFFF) == memaddr);
  assert(memaddr % kWordSize == 0);  // kWordSize aligned memory.
  return Word(ISA::LOAD_RI | dest << 6 | memaddr << 11);
}

Word LoadRR(uint32_t dest, uint32_t src) {
  return LoadIX(dest, src, 0);
}

Word LoadIX(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::LOAD_IX | dest << 6 | src << 11  | offset << 16);
}

Word LoadIXR(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(offset < kRegCount);
  return Word(ISA::LOAD_IXR | dest << 6 | src << 11  | offset << 16);
}

Word LoadPI(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::LOAD_PI | dest << 6 | src << 11  | offset << 16);
}

Word LoadIP(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::LOAD_IP | dest << 6 | src << 11  | offset << 16);
}

Word StorRI(uint32_t memaddr, uint32_t src) {
  assert((memaddr & 0x1FFFFF) == memaddr);
  assert(memaddr % kWordSize == 0);  // kWordSize aligned memory.
  assert(src < kRegCount);
  return Word(ISA::STOR_RI | src << 6 | memaddr << 11);
}

Word StorRR(uint32_t dest, uint32_t src) {
  return StorIX(dest, src, 0);
}

Word StorIX(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::STOR_IX | dest << 6 | src << 11  | offset << 16);
}

Word StorPI(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::STOR_PI | dest << 6 | src << 11  | offset << 16);
}

Word StorIP(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  offset = offset & 0xFFFF;
  return Word(ISA::STOR_IP | dest << 6 | src << 11  | offset << 16);
}

Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::ADD_RR | dest << 6 | op1 << 11 | op2 << 16);
}

Word AddRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::ADD_RI | dest << 6 | op1 << 11 | value << 16);
}

Word SubRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::SUB_RR | dest << 6 | op1 << 11 | op2 << 16);
}

Word SubRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::SUB_RI | dest << 6 | op1 << 11 | value << 16);
}

Word Jmp(uint32_t memaddr) {
  memaddr = memaddr & 0x3FFFFFF;
  return Word(ISA::JMP | memaddr << 6);
}

Word Jne(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JNE | reg << 6 | memaddr << 11);
}

Word Jeq(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JEQ | reg << 6 | memaddr << 11);
}

Word Jgt(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JGT | reg << 6 | memaddr << 11);
}

Word Jge(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JGE | reg << 6 | memaddr << 11);
}

Word Jlt(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JLT | reg << 6 | memaddr << 11);
}

Word Jle(uint32_t reg, uint32_t memaddr) {
  assert(reg < kRegCount);
  memaddr = memaddr & 0x1FFFFF;
  return Word(ISA::JLE | reg << 6 | memaddr << 11);
}

Word CallI(uint32_t memaddr) {
  memaddr = memaddr & 0x3FFFFFF;
  return Word(ISA::CALLI | memaddr << 6);
}

Word CallR(uint32_t dest) {
  assert(dest < kRegCount);
  return Word(ISA::CALLR | dest << 6);
}

Word Ret() {
  return Word(ISA::RET);
}

Word AndRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::AND_RR | dest << 6 | op1 << 11 | op2 << 16);
}

Word AndRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::AND_RI | dest << 6 | op1 << 11 | value << 16);
}

Word OrrRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::ORR_RR | dest << 6 | op1 << 11 | op2 << 16);
}

Word OrrRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::ORR_RI | dest << 6 | op1 << 11 | value << 16);
}

Word XorRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::XOR_RR | dest << 6 | op1 << 11 | op2 << 16);
}

Word XorRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::XOR_RI | dest << 6 | op1 << 11 | value << 16);
}

Word LslRR(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(offset < kRegCount);
  return Word(ISA::LSL_RR | dest << 6 | src << 11 | offset  << 16);
}

Word LslRI(uint32_t dest, uint32_t src, uint32_t value) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::LSL_RI | dest << 6 | src << 11 | value  << 16);
}

Word LsrRR(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(offset < kRegCount);
  return Word(ISA::LSR_RR | dest << 6 | src << 11 | offset  << 16);
}

Word LsrRI(uint32_t dest, uint32_t src, uint32_t value) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::LSR_RI | dest << 6 | src << 11 | value  << 16);
}

Word AsrRR(uint32_t dest, uint32_t src, uint32_t offset) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  assert(offset < kRegCount);
  return Word(ISA::ASR_RR | dest << 6 | src << 11 | offset  << 16);
}

Word AsrRI(uint32_t dest, uint32_t src, uint32_t value) {
  assert(dest < kRegCount);
  assert(src < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::ASR_RI | dest << 6 | src << 11 | value  << 16);
}

Word MulRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::MUL_RR | dest << 6 | op1 << 11 | op2  << 16);
}

Word MulRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::MUL_RI | dest << 6 | op1 << 11 | value  << 16);
}

Word DivRR(uint32_t dest, uint32_t op1, uint32_t op2) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::DIV_RR | dest << 6 | op1 << 11 | op2  << 16);
}

Word DivRI(uint32_t dest, uint32_t op1, uint32_t value) {
  assert(dest < kRegCount);
  assert(op1 < kRegCount);
  value = value & 0xFFFF;
  return Word(ISA::DIV_RI | dest << 6 | op1 << 11 | value  << 16);
}

Word MullRR(uint32_t destH, uint32_t destL, uint32_t op1, uint32_t op2) {
  assert(destH < kRegCount);
  assert(destL < kRegCount);
  assert(op1 < kRegCount);
  assert(op2 < kRegCount);
  return Word(ISA::MULL_RR | destH << 6 | destL << 11 | op1 << 16 | op2 << 21);
}

Word Wfi() {
  return Word(ISA::WFI);
}

}  // namespace gvm
