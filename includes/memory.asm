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

; Memory library
.library

.section text

; ==== Memset. Sets a memory region to a specific value.
@func set:
    ; r1: start address
    ; r2: size in words
    ; r3: value to set.
    strip [r1, 4], r3
    sub r2, r2, 1
    jgt r2, set
    ret
@endf set

; ==== Memset2. Same as set but assumes size is a multiple of 2 words.
@func set2:
    ; r1: start address
    ; r2: size in words. MUST BE A MULTIPLE OF 2 WORDS.
    ; r3: value to set.
    stpip [r1, 8], r3, r3
    sub r2, r2, 2
	jgt r2, set2
	ret
@endf set2

; ==== Memset4. Same as set but assumes size is a multiple of 4 words.
@func set4:
    ; r1: start address
    ; r2: size in words. MUST BE A MULTIPLE OF 2 WORDS.
    ; r3: value to set.
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    sub r2, r2, 4
	jgt r2, set4
	ret
@endf set4


; ==== Memset32. Same as set but assumes size is a multiple of 32 words.
@func set32:
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
    jgt r2, set32
	ret
@endf set32

; ==== Memcopy. Copies the contents of one region of memory to another.
; Does not handle overlap.
@func copy:
    ; r1: start to-address
    ; r2: start from:address
    ; r3: size in words.
    ; r4: local variable for copying memory.
    ldrip r4, [r2, 4]
    strip [r1, 4], r4
    sub r3, r3, 1
    jgt r3, copy
    ret
@endf copy

; ==== Memcopy2. Same as copy but assumes size is a multiple of 2 words.
; Dones not handle overalp.
@func copy2:
	; r1: start to-addres
	; r2: start from-addres
	; r3: size in words.
	; r24, r25: local variable for copying memory.
	ldpip r24, r25, [r2, 8]
	stpip [r1, 8], r24, r25
	sub r3, r3, 2
	jgt r3, copy2
	ret
@endf copy2

; ==== Memcopy4. Same as copy but assumes size is a multiple of 2 words.
; Dones not handle overalp.
@func copy4:
	; r1: start to-addres
	; r2: start from-addres
	; r3: size in words.
	; r24, r25: local variable for copying memory.
	ldpip r24, r25, [r2, 8]
	stpip [r1, 8], r24, r25
	ldpip r24, r25, [r2, 8]
	stpip [r1, 8], r24, r25
	sub r3, r3, 4
	jgt r3, copy4
	ret
@endf copy4

; ==== Memcopy32. Same as copy but assumes size is a multiple of 32 words.
; Does not handle overlap.
@func copy32:
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
    jgt r3, copy32
    ret
@endf copy32

@func alloc:
    ; r0: returns address of memory. if == 0 no memory was available.
    ; r1: Size in bytes to allocate.
    ; r2: pointer to heap break address. Gets updated with the new limit
    ; r3: heap start
    ; r4: heap end

	; If r1 <= 0, we have an error.
	jle r1, no_memory

    ; We have positive bytes. Add the header size to the number of bytes.
    add r1, r1, mh_size

    ; Then, convert the amount of bytes to the amount of pages.
    lsr r0, r1, memory_page_shift

    ; Now convert back to bytes. If it is smaller,  add an extra page.
    lsl r0, r0, memory_page_shift
    sub r0, r1, r0
    jeq r0, ok_page_count

    ; Smaller, so adding a page to r1
	mov r0, 1
	lsl r0, r0, memory_page_shift
	add r1, r0, r1

ok_page_count:
    ; Because we create a linked list of pointers, we need to walk the list in order
    ; to find available heap memory.

    ; Copy heap start to r5 because r3 is constant and will be used by brk.
    mov r5, r3

walk_list:
	; Ok, we are at the start of a header. We need to check a few things:
	; 1) If mh_bytes == 0 then we allocate a new page.
	ldri r6, [r5, mh_bytes]
	jeq r6, allocate_page

	; 2) mh.size < 0 is a free page. We now check if r1 <= -mh.size
	mul r6, r6, -1
	sub r7, r6, r1
	jlt r7, next_or_allocate

	; Ok, bytes are available! Write -mh.size back and return the valid address.
	; TODO(icc): Shorten the page in case it's bigger.
	stri [r5, mh_bytes], r6

	; Memory block starts right after the header.
	add r0, r5, mh_size
	ret

next_or_allocate:
	; 3) Either mh.next is valid and we loop to the next block or it is invalid
	; and we need to allocate a new page.
	ldri r6, [r5, mh_next]
	jeq r6, allocate_page

	; mh.next is valid. Copy it to r5 and loop.
	mov r5, r6
	jmp walk_list

allocate_page:
	; Finally we know that no page is available in the list, so we need to
	; allocate a new one.

	; We need to get the current limit. For that, we call brk(0).
	; Copy bytes to r5
	mov r5, r1

	; Set r1 to 0
	mov r1, 0

	; Call brk(0)
	call brk

	; r0 now has the current heap address. Now we need to allocate memory so
    ; that this block become valid.

	; Copy the block to r7
	mov r7, r0

	; Copy the number of bytes to request to r1
	mov r1, r5

	; Call brk(bytes)
	call brk

	; r0 now has either a valid new memory addres or < 0. If it is < 0 then no
	; memory was allocated

	jeq r0, no_memory

	; Ok, we got memory! Setup the header and return the start of the block.
	stri [r7, mh_bytes], r1
	stri [r7, mh_next], r0

	; Set r0 mh_bytes to 0 so we know this is the point that we need to allocate.
	stri [r0, mh_bytes], rZ
	stri [r0, mh_next], rZ

	; memory block starts right after header.
	add r0, r7, mh_size
	ret

no_memory:
    mov r0, 0
    ret

@endf alloc

; ==== Brk. Adjustes heap break limit.
@infunc brk:
    ; r0: returns break limit. If < 0, was unable to allocate.
    ; r1: Size in bytes. If 0, returns address of current heap break.
    ; r2: pointer to heap break address. Gets updated with the new limit.
    ; r3: heap start addres.
    ; r4: heap end address.

    ; Load the heap break limit.
    ldr r0, [r2]
    jne r1, brk_limit

    ; Size is 0. Just return the heap break.
    ret

brk_limit:
    ; Compute the next break limit.
    add r0, r0, r1

    ; If size (r1) > 0, caller wants more memory. Need to check upper limit.
    jgt r1, more_memory

    ; if size (r1) < 0, caller wants to return memory. Need to check lower limit
    sub r5, r0, r3
    jge r5, done

    ; Caller wants to return more memory than available. We will just cap
    ; it to the lower limit and return.
    mov r0, r3
    jmp done

more_memory:
    ; If upper limit exceeded, return an error.
    sub r5, r4, r0
    jlt r5, no_memory

done:
    ; We have a new heap break and it is stored in r0. Save it and return.
    str [r2], r0
    ret

no_memory:
    ; We have crossed a memory limit. Return an error.
    mov r0, 0
    ret
@endf brk

