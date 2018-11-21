#ifndef _GVM_ISA_H_
#define _GVM_ISA_H_

#include <cstdint>

namespace gvm {

constexpr uint32_t kWordSize = 4;
constexpr uint32_t kRegCount = 14 + 2;  // 14 general purpose + sp + fp.

enum ISA {
    HALT    = 0,
    NOP     = 1,
    JMP     = 2,
    MOV_RR  = 3,
    MOV_RI  = 4,
    STOR_RI = 5,
    STOR_RR = 6,
    STOR_IX = 7,
    LOAD_RI = 8,
    LOAD_RR = 9,
    LOAD_IX = 10,
    ADD_RR  = 11,
    ADD_RI  = 12,
    SUB_RR  = 13,
    SUB_RI  = 14,
    JNE     = 15,
    JEQ     = 16,
    JGT     = 17,
    JGE     = 18,
    JLT     = 19,
    JLE     = 20,
    CALLI   = 21,
    CALLR   = 22,
    RET     = 23,
    AND_RR  = 24,
    AND_RI  = 25,
    ORR_RR  = 26,
    ORR_RI  = 27,
    XOR_RR  = 28, 
    XOR_RI  = 29,
    LSL_RR  = 30,
    LSL_RI  = 31,
    LSR_RR  = 32,
    LSR_RI  = 33,
    ASR_RR  = 34,
    ASR_RI  = 35,
};

typedef uint32_t Word;

Word Nop();
Word Halt();
Word MovRR(uint32_t dest, uint32_t src);
Word MovRI(uint32_t dest, uint32_t value);
Word LoadRI(uint32_t dest, uint32_t memadr);
Word LoadRR(uint32_t dest, uint32_t src);
Word LoadIX(uint32_t dest, uint32_t src, uint32_t offset);
Word StorRI(uint32_t memaddr, uint32_t src);
Word StorRR(uint32_t dest, uint32_t src);
Word StorIX(uint32_t dest, uint32_t src, uint32_t offset);
Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word AddRI(uint32_t dest, uint32_t op1, uint32_t value);
Word SubRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word SubRI(uint32_t dest, uint32_t op1, uint32_t value);
Word Jmp(uint32_t memaddr);
Word Jne(uint32_t memaddr);
Word Jeq(uint32_t memaddr);
Word Jgt(uint32_t memaddr);
Word Jge(uint32_t memaddr);
Word Jlt(uint32_t memaddr);
Word Jle(uint32_t memaddr);
Word CallI(uint32_t memaddr);
Word CallR(uint32_t r);
Word Ret();
Word AndRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word AndRI(uint32_t dest, uint32_t op1, uint32_t value);
Word OrrRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word OrrRI(uint32_t dest, uint32_t op1, uint32_t value);
Word XorRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word XorRI(uint32_t dest, uint32_t op1, uint32_t value);
Word LslRR(uint32_t dest, uint32_t src, uint32_t offset);
Word LslRI(uint32_t dest, uint32_t src, uint32_t value);
Word LsrRR(uint32_t dest, uint32_t src, uint32_t offset);
Word LsrRI(uint32_t dest, uint32_t src, uint32_t value);
Word AsrRR(uint32_t dest, uint32_t src, uint32_t offset);
Word AsrRI(uint32_t dest, uint32_t src, uint32_t value);
}  // namepsace gvm

#endif  // _GVM_ISA_H_
