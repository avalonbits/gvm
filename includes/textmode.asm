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

; ==== TextPutC: Prints a charcater on the screen in text mode.
.section text
@func putc:
    ; r1: x-pos
    ; r2: y-pos
    ; r3 foreground color
    ; r4: background color
    ; r5: framebuffer start.
    ; r6: Character unicode value.

    ; We calculate position in framebuffer using the formula
    ; pos(x,y) x*4 + frame_buffer + y * 100 * 4
    lsl r1, r1, 2
    mul r2, r2, 400
    add r5, r5, r1
    add r5, r5, r2

    ; In text mode, we write a word with 2 bytes for char, 1 byte for fcolor
    ; and 1 byte for bcolor. char is in r1, so we just need to write the colors.
    and r6, r6, 0xFFFF
    and r3, r3, 0xFF
    lsl r3, r3, 16
    orr r6, r6, r3

    and r4, r4, 0xFF;
    lsl r3, r3, 24
    orr r6, r6, r4

    str [r5], r6
    ret
@endf putc


