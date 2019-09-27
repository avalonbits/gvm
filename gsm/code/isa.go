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

package code

import (
	"github.com/avalonbits/gsm/parser"
)

func Nop() parser.Word {
	return parser.Word(parser.Nop)
}

func Ret() parser.Word {
	return parser.Word(parser.Ret)
}

func Halt() parser.Word {
	return parser.Word(parser.Halt)
}

func MovRR(dest, src uint32) parser.Word {
	return AddRI(dest, src, 0)
}

func MovRI(dest, value uint32) parser.Word {
	return AddRI(dest, 31, value)
}

func LoadRR(dest, src uint32) parser.Word {
	return LoadIX(dest, src, 0)
}

func LoadPC(dest, value uint32) parser.Word {
	value = value & 0x1FFFFF
	return parser.Word(parser.Load_pc) | parser.Word(dest)<<6 | parser.Word(value)<<11
}

func LoadRI(dest, value uint32) parser.Word {
	value = value & 0x1FFFFF
	return parser.Word(parser.Load_ri) | parser.Word(dest)<<6 | parser.Word(value)<<11
}

func LoadIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Load_ix) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadIXR(dest, src, offset uint32) parser.Word {
	return parser.Word(parser.Load_ixr) | parser.Word(dest)<<6 | parser.Word(src)<<11 |
		parser.Word(offset)<<16
}

func LoadIP(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Load_ip) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadPairIP(dest1, dest2, src, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return parser.Word(parser.Ldp_ip) | parser.Word(dest1)<<6 | parser.Word(dest2)<<11 |
		parser.Word(src)<<16 | parser.Word(offset)<<21
}

func LoadPI(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Load_pi) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadPairPI(dest1, dest2, src, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return parser.Word(parser.Ldp_pi) | parser.Word(dest1)<<6 | parser.Word(dest2)<<11 |
		parser.Word(src)<<16 | parser.Word(offset)<<21
}

func StorRR(dest, src uint32) parser.Word {
	return StorIX(dest, src, 0)
}

func StorRI(dest, src uint32) parser.Word {
	dest = dest & 0x1FFFFF
	return parser.Word(parser.Stor_ri) | parser.Word(src)<<6 | parser.Word(dest)<<11
}

func StorIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Stor_ix) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorPC(dest, src uint32) parser.Word {
	dest = dest & 0x1FFFFF
	return parser.Word(parser.Stor_pc) | parser.Word(src)<<6 | parser.Word(dest)<<11
}

func StorPI(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Stor_pi) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorIP(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return parser.Word(parser.Stor_ip) | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorPairPI(dest, src1, src2, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return parser.Word(parser.Stp_pi) | parser.Word(dest)<<6 | parser.Word(src1)<<11 |
		parser.Word(src2)<<16 | parser.Word(offset)<<21
}

func StorPairIP(dest, src1, src2, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return parser.Word(parser.Stp_ip) | parser.Word(dest)<<6 | parser.Word(src1)<<11 | parser.Word(src2)<<16 | parser.Word(offset)<<21
}

func AddRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Add_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AddRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Add_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func SubRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Sub_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func SubRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Sub_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func Jmp(addr uint32) parser.Word {
	addr = addr & 0x3FFFFFF
	return parser.Word(parser.Jmp) | parser.Word(addr)<<6
}

func jumpc(inst parser.OpCode, reg, addr uint32) parser.Word {
	addr = addr & 0x1FFFFF
	return parser.Word(inst) | parser.Word(reg)<<6 | parser.Word(addr)<<11
}

func Jne(reg, addr uint32) parser.Word {
	return jumpc(parser.Jne, reg, addr)
}

func Jeq(reg, addr uint32) parser.Word {
	return jumpc(parser.Jeq, reg, addr)
}

func Jgt(reg, addr uint32) parser.Word {
	return jumpc(parser.Jgt, reg, addr)
}

func Jge(reg, addr uint32) parser.Word {
	return jumpc(parser.Jge, reg, addr)
}

func Jlt(reg, addr uint32) parser.Word {
	return jumpc(parser.Jlt, reg, addr)
}

func Jle(reg, addr uint32) parser.Word {
	return jumpc(parser.Jle, reg, addr)
}

func CallI(addr uint32) parser.Word {
	addr = addr & 0x3FFFFFF
	return parser.Word(parser.Call_i) | parser.Word(addr)<<6
}

func CallR(dest uint32) parser.Word {
	return parser.Word(parser.Call_r) | parser.Word(dest)<<6
}

func AndRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.And_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func OrrRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Orr_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func XorRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Xor_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LslRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Lsl_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LsrRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Lsr_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AsrRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Asr_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func MulRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Mul_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func DivRR(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Div_rr) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AndRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.And_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func OrrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Orr_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func XorRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Xor_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LslRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Lsl_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Lsr_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return parser.Word(parser.Asr_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func MulRI(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Mul_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func DivRI(dest, op1, op2 uint32) parser.Word {
	return parser.Word(parser.Div_ri) | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func Wfi() parser.Word {
	return parser.Word(parser.Wfi)
}
