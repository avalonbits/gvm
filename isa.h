#ifndef _GVM_ISA_H_
#define _GVM_ISA_H_

#include <cstdint>

namespace gvm {

constexpr uint32_t kWordSize = 4;
constexpr uint32_t kRegCount = 14 + 2;  // 14 general purpose + sp + fp.

enum ISA {
    HALT    = 0,   // 0b00000000
    NOP     = 1,   // 0b00000001
    JMP     = 2,   // 0b00000010
    MOV_RR  = 3,   // 0b00000011
    MOV_RI  = 4,   // 0b00000100
    STOR_RI = 5,   // 0b00000101
    STOR_RR = 6,   // 0b00000110
    STOR_IX = 7,   // 0b00000111
    LOAD_RI = 8,   // 0b00001000
    LOAD_RR = 9,   // 0b00001001
    LOAD_IX = 10,  // 0b00001010
    ADD_RR  = 11,  // 0b00001011
    ADD_RI  = 12,  // 0b00001100
    SUB_RR  = 13,  // 0b00001101
    SUB_RI  = 14,  // 0b00001110
    JNE     = 15,  // 0b00001111
    JEQ     = 16,  // 0b00010000
    JGT     = 17,  // 0b00010001
    JGE     = 18,  // 0b00010010
    JLT     = 19,  // 0b00010011
    JLE     = 20,  // 0b00010100
    CALL    = 21,  // 0b00010101
    RET     = 22,  // 0b00010110
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
Word Jne(uint32_t memaddr);
Word Jgt(uint32_t memaddr);
Word Jge(uint32_t memaddr);
Word Jlt(uint32_t memaddr);
Word Jle(uint32_t memaddr);
Word Call(uint32_t memaddr);
Word Ret();
}  // namepsace gvm

#endif
