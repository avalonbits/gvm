/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GVM_ISA_H_
#define _GVM_ISA_H_

#include <cstdint>

namespace gvm {

constexpr uint32_t kWordSize = 4;
constexpr uint32_t kRegCount = 32;

enum ISA {
    NOP = 0,
    HALT,
    LOAD_RI,
    LOAD_IX,
    LOAD_PC,
    LOAD_IXR,
    LOAD_PI,
    LOAD_IP,
    LDP_PI,
    LDP_IP,
    STOR_RI,
    STOR_IX,
    STOR_PC,
    STOR_PI,
    STOR_IP,
    STP_PI,
    STP_IP,
    ADD_RR,
    ADD_RI,
    SUB_RR,
    SUB_RI,
    JMP,
    JNE,
    JEQ,
    JGT,
    JGE,
    JLT,
    JLE,
    CALLI,
    CALLR,
    RET,
    AND_RR,
    AND_RI,
    ORR_RR,
    ORR_RI,
    XOR_RR,
    XOR_RI,
    LSL_RR,
    LSL_RI,
    LSR_RR,
    LSR_RI,
    ASR_RR,
    ASR_RI,
    MUL_RR,
    MUL_RI,
    DIV_RR,
    DIV_RI,
    MULL_RR,
    WFI,
};

typedef uint32_t Word;

Word Nop();
Word Halt();
Word MovRR(uint32_t dest, uint32_t src);
Word MovRI(uint32_t dest, uint32_t value);
Word LoadRI(uint32_t dest, uint32_t memadr);
Word LoadRR(uint32_t dest, uint32_t src);
Word LoadIX(uint32_t dest, uint32_t src, uint32_t offset);
Word LoadIXR(uint32_t dest, uint32_t src, uint32_t offset);
Word LoadPI(uint32_t dest, uint32_t src, uint32_t offset);
Word LoadIP(uint32_t dest, uint32_t src, uint32_t offset);
Word StorRI(uint32_t memaddr, uint32_t src);
Word StorRR(uint32_t dest, uint32_t src);
Word StorIX(uint32_t dest, uint32_t src, uint32_t offset);
Word StorPI(uint32_t dest, uint32_t src, uint32_t offset);
Word StorIP(uint32_t dest, uint32_t src, uint32_t offset);
Word AddRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word AddRI(uint32_t dest, uint32_t op1, uint32_t value);
Word SubRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word SubRI(uint32_t dest, uint32_t op1, uint32_t value);
Word Jmp(uint32_t memaddr);
Word Jne(uint32_t reg, uint32_t memaddr);
Word Jeq(uint32_t reg, uint32_t memaddr);
Word Jgt(uint32_t reg, uint32_t memaddr);
Word Jge(uint32_t reg, uint32_t memaddr);
Word Jlt(uint32_t reg, uint32_t memaddr);
Word Jle(uint32_t reg, uint32_t memaddr);
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
Word MulRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word MulRI(uint32_t dest, uint32_t op1, uint32_t value);
Word DivRR(uint32_t dest, uint32_t op1, uint32_t op2);
Word DivRI(uint32_t dest, uint32_t op1, uint32_t value);
Word MullRR(uint32_t destH, uint32_t destL, uint32_t op1, uint32_t op2);
Word Wfi();
}  // namepsace gvm

#endif  // _GVM_ISA_H_
