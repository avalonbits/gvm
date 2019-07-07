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

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset.
interrupt_table:
  jmp benchmark  ; Reset interrupt.
  ret            ; Timer interrupt.
  ret            ; Input intterupt.

.section data

farray:   .array 0x100000  ; 1MiB array
tarray:   .array 0x100000  ; 1MiB array
size_words: .int 0x40000   ; 256Ki words.
faddr: .int farray
taddr: .int tarray
iter: .int 300

.section text

; ==== Memcopy32. Same as memcpy but assumes size is a multiple of 32 words.
memcpy32:
    ; r1: start to-address
    ; r2: start from:address
    ; r3: size in words.
    ; r24, r25: local variable for copying memory.
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    ldpip r24, r25, [r2, 8]
    stpip [r1, 8], r24, r25
    sub r3, r3, 32
    jgt r3, memcpy32
    ret

; ==== Memset32. Same as memset but assumes size is a multiple of 32 words.
memset32:
    ; r1: start address
    ; r2: size in words. MUST BE A MULTIPLE OF 32.
    ; r3: value to set.
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    sub r2, r2, 32
    jgt r2, memset32
    ret

; ===== The acutal benchmark function.
@infunc benchmark:
    ldr r0, [iter]

    ldr r1, [faddr]
    ldr r2, [size_words]
    mov r3, 0xF

    call memset32

loop:

    ldr r1, [taddr]
    ldr r2, [faddr]
    ldr r3, [size_words]

    call memcpy32
    sub r0, r0, 1
    jne r0, loop

    halt
@endf benchmark
