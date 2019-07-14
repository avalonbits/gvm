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

; Memory library
.library

.section text

; ==== Memset. Sets a memory region to a specific value.
@func memset:
    ; r1: start address
    ; r2: size in words
    ; r3: value to set.
    strip [r1, 4], r3
    sub r2, r2, 1
    jgt r2, _memset
    ret
@endf memset

; ==== Memset2. Same as memset but assumes size is a multiple of 2 words.
@func memset2:
    ; r1: start address
    ; r2: size in words. MUST BE A MULTIPLE OF 2 WORDS.
    ; r3: value to set.
    stpip [r1, 8], r3, r3
    sub r2, r2, 2
	jgt r2, _memset2
	ret
@endf memset2

; ==== Memset32. Same as memset but assumes size is a multiple of 32 words.
@func memset32:
    ; r1: start address
    ; r2: size in words. MUST BE A MULTIPLE OF 32 WORDS.
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
    jgt r2, _memset32
	ret
@endf memset32

; ==== Memcopy. Copies the contents of one region of memory to another.
; Does not handle overlap.
@func memcpy:
    ; r1: start to-address
    ; r2: start from:address
    ; r3: size in words.
    ; r4: local variable for copying memory.
    ldrip r4, [r2, 4]
    strip [r1, 4], r4
    sub r3, r3, 1
    jgt r3, _memcpy
    ret
@endf memcpy

; ==== Memcopy2. Same as memcpy but assumes size is a multiple of 2 words.
; Dones not handle overalp.
@func memcpy2:
	; r1: start to-addres
	; r2: start from-addres
	; r3: size in words.
	; r24, r25: local variable for copying memory.
	ldpip r24, r25, [r2, 8]
	stpip [r1, 8], r24, r25
	sub r3, r3, 2
	jgt r3, _memcpy2
	ret
@endf memcpy2

; ==== Memcopy32. Same as memcpy but assumes size is a multiple of 32 words.
; Does not handle overlap.
@func memcpy32:
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
    jgt r3, _memcpy32
    ret
@endf memcpy32

