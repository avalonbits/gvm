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

.library

.section data
input_value_addr: .int 0x1200404
	; struct input_buffer. Defines 32 key long circular buffer.
	.equ ib_buffer       0
	.equ ib_head        128  ; 32 * 4.
	.equ ib_tail        132
	.equ ib_struct_size 136
input_buffer: .array 136

.section text

; ==== Input interrupt handler
; The input interrupt handler adds ew key inputs to the circular buffer. If the
; buffer is full, the input buffer is overwritten.
@func input_handler:
    ; Save the contents of r0 and r2 on the stack so we don't disrupt user code.
    stppi [sp, -8], r0, r2

    ; Read the value from the input.
    ldr r2, [input_value_addr]
    ldr r2, [r2]

    ; Quit is the value 0xFFFFFFFF so adding 1 should result in 0.
    add r0, r2, 1
    jeq r0, quit

	; Save r1 and r3 to the stack.
	stppi [sp, -8], r1, r3

	; Load input buffer.
	mov r0, input_buffer

    ; Load the buffer tail and write.
	ldri r1, [r0, ib_tail]

	; Because we operate with words, we need to multiply r1 by 4.
	lsl r3, r1, 2
	add r3, r0, r3
	str [r3], r2

	; Update the tail
	add r1, r1, 1

	; If it is not 32, then just write it back and be done.
	sub r3, r1, 32
	jne r3, done

	; Need to wrap around.
	mov r1, 0

done:
	stri [r0, ib_tail], r1

    ; Input processing done. Restore registers and return.
	ldpip r1, r3, [sp, 8]
    ldpip r0, r2, [sp, 8]
    ret

quit:
    ; Quit means we want to turn of the cpu.
    halt
@endf input_handler

; === GetKey: Reads the next key from the key input buffer. It requires that
;             the input_handler is being used to process input interrupts.
@func getkey:
	; r0: returns the key read. r0 == 0 means no input available.

	; The first thing to do is to get ib_tail. This way, if an interrupt
    ; happens while we are in this function, we will reduce the chance of
	; buffer overwrite.
	mov r0, input_buffer
	ldri r2, [r0, ib_tail]
	ldri r1, [r0, ib_head]

	;  If tail != head then the buffer has keys so we can read it.
	sub r2, r2, r1
	jne r2, read_input

	; tail == head means no key in buffer. Return 0.
	mov r0, 0
	ret

read_input:
	; We read the input to r2, update ib_head then move the result to r0 to
	; return it to caller.

	; Because we operate on words, we need to multiply ib_head by 4.
	lsl r2, r1, 2

	; Now get the char at buffer[r2]
	add r2, r0, r2
	ldr r2, [r2]

	; Update ib_head and wrap if needed.
	add r1, r1, 1
	sub r3, r1, 32
	jne r3, done

	; We need to wrap back to 0.
	mov r1, rZ

done:
	stri [r0, ib_head], r1
	mov r0, r2
	ret
@endf getkey
