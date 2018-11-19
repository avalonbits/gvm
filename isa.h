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
    AND_RR  = 23,  // 0b00010111
    AND_RI  = 24,  // 0b00011000
    ORR_RR  = 25,  // 0b00011001
    ORR_RI  = 26,  // 0b00011010
    XOR_RR  = 27,  // 0b00011011
    XOR_RI  = 28,  // 0b00011100
    LSL_RR  = 29,  // 0b00011101
    LSL_RI  = 30,  // 0b00011110
    LSR_RR  = 31,  // 0b00011111
    LSR_RI  = 32,  // 0b00100000
    ASR_RR  = 33,  // 0b00100001
    ASR_RI  = 34,  // 0b00100010

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
Word Call(uint32_t memaddr);
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
