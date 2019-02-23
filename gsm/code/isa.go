package code

import (
	"github.com/avalonbits/gsm/parser"
)

const (
	nop parser.Word = iota
	mov_rr
	mov_ri
	load_rr
	load_ri
	load_ix
	stor_rr
	stor_ri
	stor_ix
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
	halt
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
	return mov_rr | parser.Word(dest)<<8 | parser.Word(src)<<12
}

func MovRI(dest, value uint32) parser.Word {
	value = value & 0xFFFFF
	return mov_ri | parser.Word(dest)<<8 | parser.Word(value)<<12
}

func LoadRR(dest, src uint32) parser.Word {
	return load_rr | parser.Word(dest)<<8 | parser.Word(src)<<12
}

func LoadRI(dest, value uint32) parser.Word {
	value = value & 0xFFFFF
	return load_ri | parser.Word(dest)<<8 | parser.Word(value)<<12
}

func LoadIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return load_ix | parser.Word(dest)<<8 | parser.Word(src)<<12 | parser.Word(offset)<<16
}

func StorRR(dest, src uint32) parser.Word {
	return stor_rr | parser.Word(dest)<<8 | parser.Word(src)<<12
}

func StoreRI(src, value uint32) parser.Word {
	value = value & 0xFFFFF
	return stor_ri | parser.Word(src)<<8 | parser.Word(value)<<12
}

func StorIX(dest, src, offset uint32) parser.Word {
	offset = offset & 0xFFFF
	return stor_ix | parser.Word(dest)<<8 | parser.Word(src)<<12 | parser.Word(offset)<<16
}

func AddRR(dest, op1, op2 uint32) parser.Word {
	return add_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func AddRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return add_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func SubRR(dest, op1, op2 uint32) parser.Word {
	return sub_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func SubRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return sub_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func Jmp(addr uint32) parser.Word {
	addr = addr & 0xFFFFFF
	return jmp | parser.Word(addr)<<8
}

func jumpc(inst parser.Word, reg, addr uint32) parser.Word {
	addr = addr & 0xFFFFF
	return inst | parser.Word(reg)<<8 | parser.Word(addr)<<12
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
	addr = addr & 0xFFFFFF
	return call_i | parser.Word(addr)<<8
}

func CallR(dest uint32) parser.Word {
	return call_r | parser.Word(dest)<<8
}

func AndRR(dest, op1, op2 uint32) parser.Word {
	return and_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func OrrRR(dest, op1, op2 uint32) parser.Word {
	return orr_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func XorRR(dest, op1, op2 uint32) parser.Word {
	return xor_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func LslRR(dest, op1, op2 uint32) parser.Word {
	return lsl_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func LsrRR(dest, op1, op2 uint32) parser.Word {
	return lsr_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func AsrRR(dest, op1, op2 uint32) parser.Word {
	return asr_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func AndRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return and_rr | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func OrrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return orr_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func XorRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return xor_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func LslRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return lsl_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func LsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return lsr_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}

func AsrRI(dest, op1, op2 uint32) parser.Word {
	op2 = op2 & 0xFFFF
	return asr_ri | parser.Word(dest)<<8 | parser.Word(op1)<<12 | parser.Word(op2)<<16
}
