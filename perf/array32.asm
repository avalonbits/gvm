; GVM Benchmark
; by Igor Cananea (icc@avalonbits.com)

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
