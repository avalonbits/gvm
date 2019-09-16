; Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
; Author: Igor Cananea <icc@avalonbits.com>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.

.bin

.org 0x0
.section text

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset.
interrupt_table:
    jmp benchmark  ; Reset interrupt.
    ret            ; Timer interrupt.
    ret            ; Input intterupt.

.org 0x1000
.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    ldr r0, [loop_size]

loop:
    ldr r1, [mem]
    add r1, r1, 1
    lsl r1, r1, 4
    call update
    str [mem], r0
    ldr r2, [mem]
    mul r2, r1, r2
    jne r0, loop
    halt
@endf benchmark

.section data
	.equ val 7

.org 0x2000
.section text
@func update:
    stppi [sp, -8], r0, r1
    add r1, r1, val
    str [mem], r1
    ldpip r0, r1, [sp, 8]
    sub r0, r0, 1
    ret
@endf update

.section data
mem: .int 0x0
loop_size: .int 0x1000000
