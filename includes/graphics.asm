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

; Bitmap graphics library.
; Graphics mode uses a 640x360 framebuffer with 32 bits color.
.library

.section data
    .equ kLineLength 2560 ; 640 * 4

.section text


; ==== HLine: draws a horizontal line on the screen.
; Linelength is 2560 bytes (640 * 32bpp)
@func hline:
    ; r1: y-pos
    ; r2: x-start
    ; r3: y-start
    ; r4: width
    ; r5: color (RGBA)
    ; r6: framebuffer start.

    ; Multiply y-pos by 2560 to get y in the frameuffer.
    mul r1, r1, kLineLength

    ; Multiply x-start and x-end by 4 for pixel size.
    lsl r3, r3, 2

width:
    lsl r8, r2, 2

    ; Now add mem start, x-start with y-pos to get the framebuffer start point.
    add r7, r1, r8
    add r7, r7, r6

line:
    ; Write pixel to framebuffer location
    strip [r7, 4], r5

    ; Increment x-start by pixel size
    add r8, r8, 4

    ; Check if we got to x-end.
    sub r6, r3, r8

    ; If not, loop back and continue.
    jne r6, line

    ; Finished line. Need to subtract one from framebuffer because we
    ; optimistically assume we need to increment.
    sub r7, r7, 1

    ; Done with one line.
    sub r4, r4, 1

    ; We increment r1 even if we are done to avoid an extra instruction.
    add r1, r1, kLineLength

    ; If still has lines, loop.
    jne r4, width

    ret
@endf hline

; ==== VLine: draws a vertical line on the screen.
@func vline:
    ; r1: x-pos
    ; r2: y-start
    ; r3: y-end
    ; r4: width
    ; r5: color (RGBA)
    ; r6: framebuffer start

    ; Multiply x-pos by 4 to get x in the framebuffer.
    lsl r1, r1, 2

    ; Multiply y-start and y-end by 2560 to get their positions.
    mul r3, r3, kLineLength

width:
    mul r8, r2, kLineLength

    ; Now add mem start, x-pos, y-start with y-end to get the framebuffer start point.
    add r7, r1, r8
    add r7, r7, r6

line:
    ; Write the pixel at the location.
    strip [r7, kLineLength], r5

    ; Increment y-start.
    add r8, r8, kLineLength

    ; Check if we got to y-end
    sub r6, r3, r8

    ; If line is not done, loop.
    jne r6, line

    ; Line is not done. Need to subtract a line from framebuffer because we
    ; optimistically assume we need to increment.
    sub r7, r7, kLineLength

    ; Done with one line.
    sub r4, r4, 1

    ; Line is still wide.
    add r1, r1, 4

    ; Loop back if we still need to print line.
    jne r4, width

    ret
@endf vline
