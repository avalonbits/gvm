; Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
; Author: Igor Cananea <icc@avalonbits.com>
;
; This program is free software: you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation, either
; version 3 of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public
; License along with this program.  If not, see
; <http://www.gnu.org/licenses/>.

.library

.section data
	.equ JMP 0x15  ; jmp instruction for registering handler.
	.equ RET 0x1e  ; ret instruction for clearing interrupt vector.


.section text
; ==== RegisterInterrrupt. Registers a function as an interrupt handler.
@func register:
	; r0: Returns 0 on failure, 1 otherise.
	; r1: interrupt value.
	; r2: absolute function address to call on interrupt. Must use <= 26 bits.

	; Handler values range from 0-255.
	jlt r1, invalid_handler
	sub r0, r1, 0xFF
	jgt r0, invalid_handler


	; We have a valid interrupt. Write the function pointer to the
	; interrupt vector.
	lsl r1, r1, 2    ; Each value in the vector is 4 bytes long.
	sub r2, r2, r1   ; We subtract the vector value because JMP is pc relative.
	lsl r2, r2, 6	 ; Make space for jump instruction
	orr r2, r2, JMP  ; jmp instruction
	str [r1], r2
	mov r0, 1
	ret

invalid_handler:
	mov r0, 0
	ret
@endf register

; ==== GetInterrrupt. Returns the function address registered for interrupt.
@func get:
	; r0: Returns 0 on failure, != 0 otherise.
	; r1: interrupt value.

	; Handler values range from 0-255.
	jlt r1, invalid_handler
	sub r0, r1, 0xFF
	jgt r0, invalid_handler

	; We have a valid interrupt. Get the pc relative funcrtion addreess, make
	; it absolute and return.
	lsl r1, r1, 2  ; Each value in the vector is 4 bytes long.
	ldr r0, [r1]
	lsr r0, r0, 6  ; Get rid of the jump instruction.
	add r0, r0, r1 ; Add the vector index offset to get the absolute adress
	ret

invalid_handler:
	mov r0, 0
	ret
@endf get
