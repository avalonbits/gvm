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

package parser

type OpCode Word

const (
	Nop OpCode = iota
	Halt
	Load_ri
	Load_ix
	Load_pc
	Load_ixr
	Load_pi
	Load_ip
	Ldp_pi
	Ldp_ip
	Stor_ri
	Stor_ix
	Stor_pc
	Stor_pi
	Stor_ip
	Stp_pi
	Stp_ip
	Add_rr
	Add_ri
	Sub_rr
	Sub_ri
	Jmp
	Jne
	Jeq
	Jgt
	Jge
	Jlt
	Jle
	Call_i
	Call_r
	Ret
	And_rr
	And_ri
	Orr_rr
	Orr_ri
	Xor_rr
	Xor_ri
	Lsl_rr
	Lsl_ri
	Lsr_rr
	Lsr_ri
	Asr_rr
	Asr_ri
	Mul_rr
	Mul_ri
	Div_rr
	Div_ri
	Mull_rr
	Wfi
)
