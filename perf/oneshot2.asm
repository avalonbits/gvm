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
; interrupts except for reset and timer.
interrupt_table:
    jmp benchmark  ; Reset interrupt.
    ret            ; Timer interrupt.
    ret            ; Input intterupt.
    ret            ; Recurring timer interrupt.
    jmp oneshot2   ; Timer2 interrupt.


.section data
oneshot2_reg: .int 0x1200414

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    ldr r0, [oneshot2_reg]
    mov r1, 9000
    str [r0], r1
    wfi
    halt
@endf benchmark

@func oneshot2:
    ldr r1, [oneshot2_reg]
    ldr r1, [r1]
    ret
@endf oneshot2
