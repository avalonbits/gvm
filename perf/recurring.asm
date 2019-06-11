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
    jmp recurring  ; Recurring timer interrupt.


.section data
recurring_reg: .int 0x1200410

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    mov r2, 0
    ldr r0, [recurring_reg]
    mov r1, 60  ; 60Hz recurring timer.
    str [r0], r1

  ; Nothing more to do in the main thread. Everything will
  ; be handled in the recurring timer interrupt handler.
loop: wfi
    jmp loop
@endf benchmark

@func recurring:
    add r2, r2, 1
    sub r3, r2, 600  ;  10 seconds.
    jeq r3, done
    ret
done:
    halt
@endf recurring
