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

.org 0x0
.section text

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset and timer.
interrupt_table:
    jmp benchmark  ; Reset interrupt.
    ret            ; Timer interrupt.
    ret            ; Input intterupt.
    ret            ; Recurring timer interrupt.
    ret            ; Timer2 interrupt.
    jmp recurring2 ; Recurring timer2 interrupt.


.section data
recurring2_reg: .int 0x1200418

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    mov r2, 0
    ldr r0, [recurring2_reg]
    mov r1, 30  ; 30Hz recurring2 timer.
    str [r0], r1

  ; Nothing more to do in the main thread. Everything will
  ; be handled in the recurring2 timer interrupt handler.
loop: wfi
      jmp loop
@endf benchmark

@func recurring2:
    add r2, r2, 1
    sub r3, r2, 270  ; 9 seconds.
    jeq r3, done
    ret
done:
    halt
@endf recurring2
