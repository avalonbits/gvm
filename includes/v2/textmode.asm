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

; Text mode library.
; Text mode works with a 100x28 character display. Each char has a foreground and
; a background colors, chosen from a 256 palette.
.library

.section data
.equ FB_WORDS   2800
.equ CURSOR   0x2588
.equ MEMSET4   0x408
.equ READKEY   0x420

framebuffer_start: .int 0x1000000
fg_color: .int 0
bg_color: .int 0
cursor_x: .int 0
cursor_y: .int 0
.section text

; ==== Init: Initializes textmode.
@func init:
	; Initialize textmode variables.
	mov r1, 15
	lsl r1, r1, 16
	str [fg_color], r1
	str [bg_color], rZ
	str [cursor_x], rZ
	str [cursor_y], rZ
	call clear

	; Loop to make sure the video was initialized.
loop:
	ldr r27, [vram_reg]
	ldr r27, [r27]
	jne r27, loop

	; Video is setup.
	ret
@endf init

; ==== Clear: Clears the screen with the current background color.
@func clear:
	; Use memset4 to clear the screen with the background color.
	ldr r1, [framebuffer_start]
	mov r2, FB_WORDS
	ldr r3, [bg_color]
	ldr r4, [MEMSET4]
	call r4

	; Now draw the cursor on the screen
	mov r0, CURSOR
	call _putc

	; Flush the memory to video controller.
	call flush
	ret
@endf clear

; ==== PutC: Prints a character on the screen at the current cursor position
;            and advances the cursor.
@func putc:
	; r0: Character unicode value.
	call _putc
	call _advance_cursor
	call flush
	ret
@endf putc

; ==== SetFgcolor: Sets the foreground color.
@func set_fgcolor:
	; r0: Foreground color.
	and r0, r0, 0xFF
	lsl r0, r0, 16
	str [fg_color], r0
	ret
@endf set_fgcolor

; ==== SetBgcolor: Sets the background color.
@func set_bgcolor:
	; r0: Background color.
	and r0, r0, 0xFF
	lsl r0, r0, 24
	str [bg_color], r0
	ret
@endf set_bgcolor

; ==== GetC: Reads a character from the input and returns it. Ignores control
;            keys and keyup events.
@func getc:
	; r0: Returns the character. Returns 0 in case no character is available,
	ldr r0, [READKEY]
	call r0
	jeq r0, done

	; Check if this is a keyup event. If it is, ignore it.
	lsr r1, r0, 31
	and r1, r1, 1
	jne r1, return_no_key

	; Check if this is a control key. If it is, ignore it.
	lsr r1, r0, 30
	and r1, r1, 1
	jne r1, return_no_key

	; We got a charater code. Just return it.
done:
	ret

return_no_key:
	mov r0, 0
	ret
@endf getc



; ========================== Internal functions ============================= ;
; ==== _PutCAt: Prints a charcater on the screen in text mode.
@infunc _putc_at:
    ; r1: x-pos
    ; r2: y-pos
    ; r3: foreground color
    ; r4: background color
    ; r5: Character unicode value.

	ldr r6, [framebuffer_start]
    ; We calculate position in framebuffer using the formula
    ; pos(x,y) x*4 + frame_buffer + y * 400
    lsl r1, r1, 2
    mul r2, r2, 400
    add r6, r6, r1
    add r6, r6, r2

    ; In text mode, we write a word with 2 bytes for char, 1 byte for fcolor
    ; and 1 byte for bcolor. char is in r1, so we just need to write the colors.
	; Here we assume the colors are already shifted to the correct position and
    ; just OR them.
    and r5, r5, 0xFFFF
    orr r5, r5, r3
    orr r5, r5, r4

    str [r6], r5
    ret
@endf _putc_at

@infunc _putc:
	; r0: Character unicode value.
	ldr r1, [cursor_x]
	ldr r2, [cursor_y]
	ldr r3, [fg_color]
	ldr r4, [bg_color]
	mov r5, r0
	call _putc_at
	ret
@endf _putc

@infunc _advance_cursor:
	; Increment the x position.
	ldr r1, [cursor_x]
	add r1, r1, 1

	; If we are not at the edge we are done.
	sub r2, r1, 100
	jlt r2, done

	; We need to go to the next line.
	mov r1, 0

	; Increment the y position.
	ldr r2, [cursor_y]
	add r2, r2, 1

	; If we are not a the edge we are done.
	sub r3, r2, 28
	jlt r3, done_y

	; For now we will keep wrapping back to the last line.
	mov r1, 0
	mov r2, 27

done_y:
	str [cursor_y], r2

done:
	str [cursor_x], r1
	mov r0, CURSOR
	call _putc
	ret
@endf _advance_cursor

.section data
vram_reg: .int 0x1200400

.section text
@infunc flush:
	mov r26, 2
	ldr r27, [vram_reg]
	str [r27], r26
	ret
@endf flush
