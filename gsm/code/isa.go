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

const (
	nop parser.Word = iota
	halt
	load_ri
	load_ix
	load_pc
	load_ixr
	load_pi
	load_ip
	ldp_pi
	ldp_ip
	stor_ri
	stor_ix
	stor_pc
	stor_pi
	stor_ip
	stp_pi
	stp_ip
	add_rr
	add_ri
	sub_rr
	sub_ri
	jmp
	jne
	jeq
	jgt
	jge
	jlt
	jle
	call_i
	call_r
	ret
	and_rr
	and_ri
	orr_rr
	orr_ri
	xor_rr
	xor_ri
	lsl_rr
	lsl_ri
	lsr_rr
	lsr_ri
	asr_rr
	asr_ri
	mul_rr
	mul_ri
	div_rr
	div_ri
	mull_rr
	wfi
)

func Nop() parser.Word {
	return nop
}

func Ret() parser.Word {
	return ret
}

func Halt() parser.Word {
	return halt
}

func MovRR(dest, src uint32) parser.Word {
	return AddRI(dest, src, 0)
}

func MovRI(dest, value uint32) parser.Word {
	return AddRI(dest, 28, value)
}

func LoadRR(dest, src uint32) parser.Word {
	return LoadIX(dest, src, 0)
}

func LoadPC(dest, value uint32) parser.Word {
	value = value & 0x1FFFFF
	return load_pc | parser.Word(dest)<<6 | parser.Word(value)<<11
}

func LoadRI(dest, value uint32) parser.Word {
	value = value & 0x1FFFFF
	return load_ri | parser.Word(dest)<<6 | parser.Word(value)<<11
}

func LoadIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return load_ix | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadIXR(dest, src, offset uint32) parser.Word {
	return load_ixr | parser.Word(dest)<<6 | parser.Word(src)<<11 |
		parser.Word(offset)<<16
}

func LoadIP(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return load_ip | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadPairIP(dest1, dest2, src, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return ldp_ip | parser.Word(dest1)<<6 | parser.Word(dest2)<<11 | parser.Word(src)<<16 | parser.Word(offset)<<21
}

func LoadPI(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return load_pi | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func LoadPairPI(dest1, dest2, src, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return ldp_pi | parser.Word(dest1)<<6 | parser.Word(dest2)<<11 | parser.Word(src)<<16 | parser.Word(offset)<<21
}

func StorRR(dest, src uint32) parser.Word {
	return StorIX(dest, src, 0)
}

func StorRI(dest, src uint32) parser.Word {
	dest = dest & 0x1FFFFF
	return stor_ri | parser.Word(src)<<6 | parser.Word(dest)<<11
}

func StorIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return stor_ix | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorPC(dest, src uint32) parser.Word {
	dest = dest & 0x1FFFFF
	return stor_pc | parser.Word(src)<<6 | parser.Word(dest)<<11
}

func StorPI(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return stor_pi | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorIP(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return stor_ip | parser.Word(dest)<<6 | parser.Word(src)<<11 | parser.Word(offset)<<16
}

func StorPairPI(dest, src1, src2, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return stp_pi | parser.Word(dest)<<6 | parser.Word(src1)<<11 | parser.Word(src2)<<16 | parser.Word(offset)<<21
}

func StorPairIP(dest, src1, src2, offset uint32) parser.Word {
	offset = offset & 0x7FF
	return stp_ip | parser.Word(dest)<<6 | parser.Word(src1)<<11 | parser.Word(src2)<<16 | parser.Word(offset)<<21
}

func AddRR(dest, op1, op2 uint32) parser.Word {
	return add_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AddRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return add_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func SubRR(dest, op1, op2 uint32) parser.Word {
	return sub_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func SubRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return sub_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func Jmp(addr uint32) parser.Word {
	addr = addr & 0x3FFFFFF
	return jmp | parser.Word(addr)<<6
}

func jumpc(inst parser.Word, reg, addr uint32) parser.Word {
	addr = addr & 0x1FFFFF
	return inst | parser.Word(reg)<<6 | parser.Word(addr)<<11
}

func Jne(reg, addr uint32) parser.Word {
	return jumpc(jne, reg, addr)
}

func Jeq(reg, addr uint32) parser.Word {
	return jumpc(jeq, reg, addr)
}

func Jgt(reg, addr uint32) parser.Word {
	return jumpc(jgt, reg, addr)
}

func Jge(reg, addr uint32) parser.Word {
	return jumpc(jge, reg, addr)
}

func Jlt(reg, addr uint32) parser.Word {
	return jumpc(jlt, reg, addr)
}

func Jle(reg, addr uint32) parser.Word {
	return jumpc(jle, reg, addr)
}

func CallI(addr uint32) parser.Word {
	addr = addr & 0x3FFFFFF
	return call_i | parser.Word(addr)<<6
}

func CallR(dest uint32) parser.Word {
	return call_r | parser.Word(dest)<<6
}

func AndRR(dest, op1, op2 uint32) parser.Word {
	return and_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func OrrRR(dest, op1, op2 uint32) parser.Word {
	return orr_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func XorRR(dest, op1, op2 uint32) parser.Word {
	return xor_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LslRR(dest, op1, op2 uint32) parser.Word {
	return lsl_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LsrRR(dest, op1, op2 uint32) parser.Word {
	return lsr_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AsrRR(dest, op1, op2 uint32) parser.Word {
	return asr_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func MulRR(dest, op1, op2 uint32) parser.Word {
	return mul_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func DivRR(dest, op1, op2 uint32) parser.Word {
	return div_rr | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AndRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return and_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func OrrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return orr_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func XorRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return xor_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LslRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return lsl_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func LsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return lsr_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func AsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return asr_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func MulRI(dest, op1, op2 uint32) parser.Word {
	return mul_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func DivRI(dest, op1, op2 uint32) parser.Word {
	return div_ri | parser.Word(dest)<<6 | parser.Word(op1)<<11 | parser.Word(op2)<<16
}

func Wfi() parser.Word {
	return wfi
}
