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


; String manipulation library.
.library

.section text

; ==== StrLen: Returns the length of a string.
@func length:
	; r0: returns length of the string.
	; r1: ptr to start of string.
	; r2, r3, r4: vars.

	mov r0, 0
	mov r4, r1

loop:
	; Load the next character.
	ldr r2, [r4]

	; Chars are 16 bit, so check if the first half-word is the null char.
	and r3, r2, 0xFFFF
	jeq r3, end_half

	; Nope. Check now the next half
	lsr r2, r2, 16
	and r3, r2, 0xFFFF
	jeq r3, end

	; Neither char was the null char. Add 2 to length and load the next word.
	add r0, r0, 2
	add r4, r4, 4
	jmp loop

end:
	; Word had one char and a null char. so add 1 to length.
	add r0, r0, 1
end_half:
	ret
@endf length


