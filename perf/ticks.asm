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


.section data
timer_reg: .int 0x1200408
b_loop: .int 100000

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    ldr r2, [b_loop]
     ldr r0, [timer_reg]
loop:
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
     nop
    ldr r1, [r0]
    nop
    ldr r1, [r0]
    sub r2, r2, 20
    jne r2, loop
@endf benchmark

    halt
