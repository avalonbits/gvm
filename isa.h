#ifndef _GVM_ISA_H_
#define _GVM_ISA_H_

#include <cstdint>

namespace gvm {

constexpr uint32_t kWordSize = 4;
constexpr uint32_t kRegCount = 16;

enum ISA {
    HALT    = 0,   // 0b00000000
    NOP     = 1,   // 0b00000001
    JMP     = 2,   // 0b00000010
    MOV_RR  = 3,   // 0b00000011
    MOV_RI  = 4,   // 0b00000100
    STOR_RI = 5,   // 0b00000100
    STOR_RR = 6,   // 0b00000101
    STOR_IX = 7,   // 0b00000111
    LOAD_RI = 8,   // 0b00001001
    LOAD_RR = 9,   // 0b00001010
    LOAD_IX = 10,  // 0b00001011
    ADD_RR  = 11,  // 0b00001100
    ADD_RI  = 12,  // 0b00001101
    SUB_RR  = 13,  // 0b00001110
    SUB_RI  = 14,  // 0b00001111
};

typedef uint32_t Word;

Word Nop();
Word Halt();
Word MovRR(uint32_t dest, uint32_t src);
Word MovRI(uint32_t dest, uint32_t value);
Word LoadRI(uint32_t dest, uint32_t memadr);
Word LoadRR(uint32_t dest, uint32_t src);
Word StorRI(uint32_t memaddr, uint32_t src);
Word StorRR(uint32_t dest, uint32_t src);
Word StorIX(uint32_t dest, uint32_t offset, uint32_t src);
Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word SubRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word Jmp(uint32_t memaddr);

}  // namepsace gvm

#endif
