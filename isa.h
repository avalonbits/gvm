#ifndef _GVM_ISA_H_
#define _GVM_ISA_H_

#include <cstdint>

namespace gvm {

constexpr uint32_t kWordSize = 4;
constexpr uint32_t kRegCount = 16;

enum  ISA {
	HALT    = 0,  // 0b00000000
	NOP     = 1,  // 0b00000001
	JMP     = 2,  // 0b00000010
	STOR_RR = 4,  // 0b00000100
	STOR_RI = 5,  // 0b00000101
	STOR_RA = 6,  // 0b00000110
	STOR_IX = 7,  // 0b00000111
	LOAD_RR = 8,  // 0b00001000
	LOAD_RI = 9,  // 0b00001001
	LOAD_RA = 10, // 0b00001010
	LOAD_IX = 11, // 0b00001011
	ADD_RR  = 12, // 0b00001100
	ADD_RI  = 13, // 0b00001101
	SUB_RR  = 14, // 0b00001110
	SUB_RI  = 15, // 0b00001111
};

typedef uint32_t Word;

Word Nop();
Word Halt();
Word LoadRR(uint32_t dest, uint32_t src);
Word LoadRI(uint32_t dest, uint32_t value);
Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2);

}  // namepsace gvm

#endif
