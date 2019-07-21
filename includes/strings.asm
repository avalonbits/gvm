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

; ==== Itoa: Converts a signed integer to a string.
@func itoa:
	; r1: number to convert
	; r2: pointer to string. Worst case must be 24 bytes long (22 bytes for digits
	;     + null terminator and 2 more bytes for memory alignment).
	mov r5, 0
	mov r6, r2
loop:
	; Buffer to store digits before writing to memory.
	mov r4, 0

	; The algorithm: keep diving by 10 until the div result is 0.
	div r0, r1, 10

	; Now, multiply the result by ten and subtract from r1 to get mod.
	mul r3, r0, 10
	sub r1, r1, r3

	; r1 contains the mod (the current digit) while r0 contains the div
	; (the next digit). We add 0x30 to the number, mask for the first byte then
	; write to r4;
	add r1, r1, 0x30
	orr r4, r4, r1

	; Increment the digit counter.
	add r5, r5, 1

	; If this is the last digit, then we are done
	jne r0, next_digit

	; We are done and only used half the word, so the null terminator is already
	; in r4. Just write it and jump to reverse.
	str [r6], r4
	sub r3, r5, 1
	jeq r3, done
	jmp reverse

next_digit:
	; We not done. Repeat the algorithm for r0.
	mov r1, r0

	; The algorithm: keep diving by 10 until the div result is 0.
	div r0, r1, 10

	; Now, multiply the result by ten and subtract from r1 to get mod.
	mul r3, r0, 10
	sub r1, r1, r3

	; r1 contains the mod (the current digit) while r0 contains the div
	; (the next digit). We add 0x30 to the number, mask for the first byte then
	; write to r4;
	add r1, r1, 0x30

	; Since this is the second digit in the loop, we need to shift it 16bits
	; before oring.
	lsl r1, r1, 16
	orr r4, r4, r1

	; Increment the digit counter.
	add r5, r5, 1

	; We have a full word to write. Write it and update the string pointer..
	strip [r6, 4], r4

	; If r0 != 0 then we still have numbers to process.
	mov r1, r0
	jne r1, loop

	; It is the last digit, so we need to write a null terminator before going
	; to reverse.
	str [r6], rZ

reverse:
	; To reverse the string, we have pointers to start and end and then swap the
	; values until either start == end or start == end - 1.
	; At this point, r2 has ptr to start of string, r6 is ptr to end of string and
	; r5 is the number of digits. Because we can't modify r2, we store of copy of
	; r2 into r4 and work with that.
	mov r4, r2

	; If the number of digits is even, then end starts a word earlier and the
	; algorithm is different.
	and r0, r5, 0x1
	jne r0, odd_reverse

	; When number of digits is even, we swap lower bits with high bits.
	sub r6, r6, 4

even_reverse:
	ldr r0, [r4]

	; If this is the last word then swapping is done differently.
	sub r3, r6, r4
	jeq r3, simple_reverse
	jlt r3, done

	; Not last word, so let's do the swap.
	ldr r1, [r6]

	; first digits.
	lsr r3, r0, 16
	lsl r3, r3, 16
	lsr r5, r1, 16
	orr r5, r3, r5
	str [r4], r5

	lsl r3, r0, 16
	lsl r5, r1, 16
	lsr r5, r5, 16
	orr r5, r5, r3
	str [r6], r5

	; second digits
	ldr r0, [r4]
	ldr r1, [r6]

	lsr r3, r0, 16
	lsr r5, r1, 16
	lsl r5, r5, 16
	orr r5, r3, r5
	strip [r6, -4], r5

	lsl r3, r1, 16
	lsl r5, r0, 16
	lsr r5, r5, 16
	orr r5, r3, r5
	strip [r4, 4], r5
	jmp even_reverse

simple_reverse:
	lsr r1, r0, 16
	lsl r0, r0, 16
	orr r0, r0, r1
	str [r4], r0
	ret

odd_reverse:
	; In odd reverse, the last word has a digit and a null terminator. this means
	; we always swap digits matching byte order. We use r0 and r1 to start current
	; digits and r3 as a temp for swap.
	ldr r0, [r4]
	ldr r1, [r6]

	; r3 will now contain the new value of r0
	lsl r3, r0, 16
	lsr r3, r3, 16
	lsr r5, r1, 16
	lsl r5, r5, 16
	orr r3, r5, r3
	strip [r6, -4], r3

	; r0 now needs to be clear in its bottom bits so we can copy the swapped one.
	lsr r3, r0, 16
	lsl r3, r3, 16
	orr r3, r3, r1
	str [r4], r3

	; r4 and r6 are the same, we are done.
	sub r3, r6, r4
	jeq r3, done

	; Still has digits, swap them.
	ldr r0, [r4]
	ldr r1, [r6]

	lsr r3, r0, 16
	lsl r3, r3, 16
	lsl r5, r1, 16
	lsr r5, r5, 16
	orr r3, r5, r3
	str [r6], r3

	lsr r3, r1, 16
	lsl r3, r3, 16
	lsl r5, r0, 16
	lsr r5, r5, 16
	orr r3, r5, r3
	strip [r4, 4], r3

	sub r3, r6, r4
	jne r3, odd_reverse

done:
	ret
@endf itoa

