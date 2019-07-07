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


; ===== The acutal benchmark function.
@func fact:
    ; r0: where we return the result.
    ; r1: the number we want to call factorial on.
    mov r0, r1
    jle r0, base_done
    sub r0, r0, 1
    jeq r0, base_done

    strpi [sp, -4], r1
    sub r1, r1, 1
    call fact
    ldrip r3, [sp, 4]
    mul r0, r0, r3
    ret

base_done:
    mov r0, 1
    ret
@endf fact

.section data
func_calls: .int 0x1

.section text
@func benchmark:
    ldr r10, [func_calls]

loop:
    mov r1, 12
    call fact
    sub r10, r10, 1
    jne r10, loop
    halt
@endf benchmark
