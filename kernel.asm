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

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
    jmp reset_handler     ; Reset interrupt.
	ret                   ; Timer1 interrupt.
	ret                   ; Input handler.
	ret                   ; Video refresh handler (recurring timer1).
	ret                   ; Timer2 interrupt.
	ret                   ; Recurring timer2 interrupt.
	ret                   ; Video interrupt.

.org 0x400
.section data
; ===== Kernel function table.

.org 0x2400
.section text
; ==== Reset interrupt handler.
reset_handler:
    ; Now jump to main kernel code.
    jmp KERNEL_MAIN

KERNEL_MAIN:
	halt
