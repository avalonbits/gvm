; GVM Kernel
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
    jmp reset_handler     ; Reset
    ret                   ; Oneshot timer handler
    jmp input_handler     ; Input handler
    jmp recurring_handler ; Recurring timer handler

.org 0x80
.section data
vram_reg:   .int 0x1200400
vram_start: .int 0x101F000

; This should be initalized to the last available memory byte.
user_stack_end_addr: .int 0x0
kernel_stack_end_addr: .int kernel_stack_end

.section text

; ==== Reset interrupt handler.
reset_handler:
	; When the cpu starts, sp is pointing to the end of user memory.
	; We will save that so that when a user program is loaded, we
	; can set its stack correctly.
	str [user_stack_end_addr], sp

	; Now set the kernel stack pointer.
	ldr sp, [kernel_stack_end_addr]

    ; Clear input register
    ldr r1, [input_value_addr]
    str [r1], rZ

    ; Clear user input vector address.
    str [input_jump_addr], rZ

    ; Now jump to main kernel code.
    jmp KERNEL_MAIN

.section data
input_value_addr: .int 0x1200404
input_jump_addr:  .int 0

.section text
; ==== Input handler
@func input_handler:
    ; Save the contents of r0 and r1 on the stack so we don't disrupt user code.
    stppi [sp, -8], r0, r1

    ; Read the value from the input.
    ldr r0, [input_value_addr]
    ldr r0, [r0]

    ; Quit is the value 0xFFFFFFFF so adding 1 should result in 0.
    add r1, r0, 1
    jeq r1, quit

    ; Load the user jump address. If it's != 0, call it.
    ldr r1, [input_jump_addr]
    jeq r1, done
    call r1

done:
    ; Input processing done. Restore restore r1 and r0 and return.
    ldpip r0, r1, [sp, 8]
    ret

quit:
    ; Quit means we want to turn of the cpu.
    halt
@endf input_handler

.section data
display_update: .int 0x0
should_update: .int 0x1
console_addr: .int 0
fb_size_words: .int 230400

.section text
@func recurring_handler:
    ; Save context.
    strpi [sp, -4], r0
    stppi [sp, -8], r1, r2
    stppi [sp, -8], r3, r4

    ; Call display update if set.
    ldr r0, [display_update]
    jeq r0, done
    call r0

    ; If !should_update, we are done.
    ldr r0, [should_update]
    jeq r0, done

    ; Ok, need to update. But first, set should_update == false.
    str [should_update], rZ

    ldr r0, [console_addr]
    call console_flush

done:
    ldpip r3, r4, [sp, 8]
    ldpip r1, r2, [sp, 8]
    ldrip r0, [sp, 4]
    ret
@endf recurring_handler

; ==== Memory handling functions.
.section data
memory_page_size: .int 128 ; bytes per page.

	; struct memory_header
	.equ page_count 0
	.equ is_free 4
	.equ next_header_ptr 8
	.equ memory_header_size 12

.section text
; ==== Brk. (De)Allocates memory from the heap.
@infunc brk:
	; r0: returns break limit. If < 0, was unable to allocate.
	; r1: Size in words. If 0, returns address of current heap break.
	; r2: pointer to heap break address. Gets updated with the new limit.
	; r3: heap start addres.
	; r4: heap end address.

	; Load the heap break limit.
	ldr r0, [r2]
	jne r1, check_size

	; Size is 0. Just return the heap break.
	ret

check_size:
	; Add the size to the heap break.
	lsl r1, r1, 2  ; words * 4 == bytes.
	add r0, r0, r1

	; If size (r1) > 0, caller wants more memory. Need to check upper limit.
	jgt r1, check_available

	; if size (r1) < 0, caller wants to return memory. Need to check lower limit
	sub r3, r0, r3
	jge r3, done

	; Caller wants to return more memory than available. We will just cap
	; it to the lower limit and return.
	mov r0, r3
	jmp done
	

check_available:
	; If upper limit exceeded, return an error.
	sub r4, r4, r0
	jlt r4, no_memory

done:
	; We have a new heap break and it is stored in r0. Save it and return.
	str [r2], r0
	ret

no_memory:
	; We have crossed a memory limit. Return an error.
	mov r0, -1
	ret
@endf brk

.section data
kernel_heap_lower_limit: .int kernel_heap_start
kernel_heap_curr_limit: .int kernel_heap_start
ptr_kernel_heap_curr_limit: .int kernel_heap_curr_limit
.section text
; ==== KBrk: Allocates memory from the kernel heap.
@infunc kbrk:
	; r0: returns break limit. If < 0, was unable to allocate.
	; r1: Size in words. If 0, returns address o current break.
	ldr r2, [ptr_kernel_heap_curr_limit]
	ldr r3, [kernel_heap_lower_limit]

	; The end of heap area is the current stack limit.
	mov r4, sp
	sub r4, r4, 4 ; to account for the call to brk.
	call brk
	ret 
@endf kbrk

; ==== UBrk: Allocates memory from the user heap.
@func ubrk:
@endf ubrk

; ==== Memset. Sets a memory region to a specific value.
memset:
    ; r1: start address
    ; r2: size in words
    ; r3: value to set.
    strip [r1, 4], r3
    sub r2, r2, 1
    jgt r2, memset
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

; ==== Memcopy. Copies the contents of one region of memory to another.
; Does not handle overlap.
memcpy:
    ; r1: start to-address
    ; r2: start from:address
    ; r3: size in words.
    ; r4: local variable for copying memory.
    ldrip r4, [r2, 4]
    strip [r1, 4], r4
    sub r3, r3, 1
    jgt r3, memcpy
    ret

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

; ====== Text mode functions and data.

.section text

; ==== PutS: Prints a string on the screen.
@func puts:
    ; r1: x-pos start.
    ; r2: y-pos start.
    ; r3: foreground color.
    ; r4: background color.
    ; r5: frame buffer address.
    ; r6: Address to string start.
	; r7: pointer to function that can print character.

    ; We copy the start addres to r20 because we will use r6 as the actual
    ; char value to print with putc.
    mov r20, r6

loop:
    ; Chars in string are 16-bit wide. So we need to AND and shift.
    ldr r22, [r20]
    and r6, r22, 0xFFFF
    jeq r6, done

    ; Save context in stack before calling putc.
    stppi [sp, -8], r1, r2
    stppi [sp, -8], r3, r4
    stppi [sp, -8], r5, r7

    call r7

    ; Restore context.
    ldpip r5, r7, [sp, 8]
    ldpip r3, r4, [sp, 8]
    ldpip r1, r2, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Next char in same word
    lsr r6, r22, 16
    jeq r6, done

    stppi [sp, -8], r1, r2
    stppi [sp, -8], r3, r4
    stppi [sp, -8], r5, r7

    call r7

    ldpip r5, r7, [sp, 8]
    ldpip r3, r4, [sp, 8]
    ldpip r1, r2, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Advance to next string word.
    add r20, r20, 4
    jmp loop

done:
    ret
@endf puts


; ==== IncXY: Given an (x,y) position, moves to the next position respecting
; screen boundaries.
@func incxy:
    ; r1: x-pos
    ; r2: y-pos
    add r1, r1, 1
    sub r21, r1, 96
    jlt r21, done

    mov r1, 0
    add r2, r2, 1

done:
    ret
@endf incxy

.section data
chrom_addr: .int 0x1100000
text_colors_addr: .int text_colors

.section text

@infunc wpixel:
    and r0, r1, 1
    jeq r0, background_color

    ; Foreground.
    strip [r2, -4], r4
    ; Shift the pixel row.
    lsr r1, r1, 1
    ret

background_color:
    strip [r2, -4], r5
    ; Shift the pixel row.
    lsr r1, r1, 1
    ret
@endf wpixel

; ==== TextPutC: Prints a charcater on the screen in text mode.
.section data
text_putc_addr: .int text_putc
.section text
@func text_putc:
    ; r1: x-pos
    ; r2: y-pos
    ; r3 foreground color
    ; r4: background color
    ; r5: framebuffer start.
	; r6: Character unicode value.

	; We calculate position in framebuffer using the formula
	; pos(x,y) x*4 + fb_addr + y * 96 * 4
	lsl r1, r1, 2
	lsl r2, r2, 7
	mul r2, r2, 3
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
@endf text_putc

; ==== PutC: Prints a character on the screen.
@func putc:
    ; r1: Character unicode value
    ; r2: x-pos
    ; r3: y-pos
    ; r4: foreground color
    ; r5: background color
    ; r6: framebuffer start.

    ; To find the (x,y) position in the frame buffer, we use the formula
    ; pos(x,y) = x-pos*8*4 + fb start + y-pos * lineLength * 16.
    lsl r2, r2, 5
    add r2, r2, r6
    mul r3, r3, kLineLength
    lsl r3, r3, 4
    add r2, r2, r3

    ; And because we process the row from right to left, we need to move the
    ; start 8 pixels (32 bytes) to the right. But because this is 0 based, we
    ; need to add 31.
    add r2, r2, 31

    ; Translate colors 0-255 to their RGBA values by multiplying the value by 4
    ; and then summing it with the start of the color table.
    ldr r0, [text_colors_addr]
    lsl r4, r4, 2
    ldri r4, [r0, r4]
    lsl r5, r5, 2
    ldri r5, [r0, r5]

    ; Each character is 8x16 pixels encoded in 16 bytes with each byte being an
    ; 8 pixel row. In order to find the start of the char we multiply the char
    ; by 16 and sum it with the start of the character rom.
    ldr r0, [chrom_addr]
    lsl r1, r1, 4
    add r1, r0, r1

    ; Copy of character start.
    mov r3, r1

    ; Number of rows per word of char.
    mov r6, 4

main_loop:
    ; Load the character word
    ldr r1, [r3]

    ; Write 8 pixel per row.
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel

    ; Reposition the frame buffer on the next row.
    add r2, r2, 2592  ; 32 + 2560.

    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel

    ; Reposition the frame buffer on the next row.
    add r2, r2, 2592  ; 32 + 2560.

    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel

    ; Reposition the frame buffer on the next row.
    add r2, r2, 2592  ; 32 + 2560.

    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel
    call wpixel

    ; Reposition the frame buffer on the next row.
    add r2, r2, 2592  ; 32 + 2560.

    sub r6, r6, 1
    ; Get the next word row and loop.
    add r3, r3, 4
    jne r6, main_loop

    ret
@endf putc

; ==== Fill816: Fills an 8x16 pixels block with the same value.
@func fill816:
    ; r1: x-pos
    ; r2: y-pos
    ; r3: value to set.
    ; r4: framebuffer start.

    ; To find the (x,y) position in the frame buffer, we use the formula
    ; pos(x,y) = x-pos*8*4 + fb_start + y-pos * lineLength * 16.
    mul r1, r1, 32
    add r1, r1, r4
    mul r2, r2, 2560
    lsl r2, r2, 4
    add r1, r1, r2

    ; Set the row counter to 16.
    mov r2, 16

loop:
    ; Each pixel is 4 bytes long, so we need to write 32 bytes per row.
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    add r1, r1, 2528

    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    add r1, r1, 2528

    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    add r1, r1, 2528

    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    stpip [r1, 8], r3, r3
    add r1, r1, 2528

    ; Check if we are done.
    sub r2, r2, 4
    jne r2, loop

    ; We are done.
    ret
@endf fill816

; ==== FlushVideo: tells the video controller that it can copy the framebuffer
; to its own memory.
@func flush_video:
    ldr r27, [vram_reg]
    str [r27], r26
    ret
@endf flush_video

; ==== WaitVideo: waits for the video controller to signal that the framebuffer
; available for writing.
@func wait_video:
    ldr r27, [vram_reg]
    ldr r27, [r27]
    jne r27, wait_video
    ret
@endf wait_video

; ======================== User interface code =======================

.section data
    ; struct sbuf
    .equ sbuf_start 0
    .equ sbuf_end 4
    .equ sbuf_size 8

    ; struct console
    .equ console_sbuf 0
    .equ console_fcolor 8
    .equ console_bcolor 12
    .equ console_cursor_x 16
    .equ console_cursor_y 20
    .equ console_size 24

.section text

@func sbuf_init:
    ; r0: ptr to start of sbuf
    ; r1: ptr to start of frame buffer.
    ; r2: ptr to last line of frame buffer.
    stri [r0, sbuf_start], r1
    stri [r0, sbuf_end], r2
    ret
@endf sbuf_init

@func console_init:
    ; r0: ptr to start of console
    ; r1: ptr to start of frame buffer
    ; r2: ptr to last line of frame buffer
    ; r3: fcolor
    ; r4: bcolor

    ; Init screen buffer.
    call sbuf_init

    ; Init console.
    stri [r0, console_fcolor], r3
    stri [r0, console_bcolor], r4
    stri [r0, console_cursor_x], rZ
    stri [r0, console_cursor_y], rZ

    ret
@endf console_init

@func console_flush:
    ; Copy sbuf to vram. First, get the number of bytes.
    ldri r1, [r0, sbuf_start]
    ldri r2, [r0, sbuf_end]
    sub r3, r2, r1

	; Convert to words.
    lsr r3, r3, 2

	; Load vram start and sbuf start.
    ldr r1, [vram_start]
    ldri r2, [r0, sbuf_start]

	; Before we copy, we need to wait for video to be ready.
	call wait_video

	; Copy to vram.
    call memcpy32

	; Flush using text mode.
	mov r26, 2
    call flush_video

	; We are done.
    ret
@endf console_flush

@func console_set_cursor:
    ; if x < 0, set it to 0.
    jge r1, x_95
    mov r1, 0

x_95:
    ; if x >= 96, set it to 95
    sub r3, r1, 96
    jlt r3, y
    mov r3, 95

y:
    ; if y < 0, set it to 0
    jge r2, y_26
    mov r2, 0

y_26:
    ; if y >= 27, set it to 26
    sub r3, r2, 27
    jlt r3, done
    mov r2, 26

done:
    stri [r0, console_cursor_x], r1
    stri [r0, console_cursor_y], r2
    ret
@endf console_set_cursor

@func console_set_color:
    ; if fcolor < 0 || > 255 set it to 15
    jlt r1, set_15
    sub r21, r1, 255
    jle r21, bcolor

set_15:
    mov r1,15

bcolor:
    ; if bcolor < 0 || bcolor > 255, set it to 0
    jlt r2, set_z
    sub r21, r2, 255
    jle r21, done

set_z:
    mov r2, 0

done:
    stri [r0, console_fcolor], r1
    stri [r0, console_bcolor], r2
    ret
@endf console_set_color

@func console_putc:
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_fcolor]
    ldri r4, [r0, console_bcolor]
    ldr r5, [fb_addr]
	call text_putc
    ret
@endf console_putc

@func console_puts:
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_fcolor]
    ldri r4, [r0, console_bcolor]
    ldr r5, [fb_addr]
	ldr r7, [text_putc_addr]
	call puts
    ret
@endf console_puts

@func console_print_cursor:
    mov r6, 0x2588
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_fcolor]
    ldri r4, [r0, console_bcolor]
    ldr r5, [fb_addr]
    call text_putc
    ret
@endf console_print_cursor

@func console_erase_cursor:
	mov r6, 0
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_bcolor]
	ldri r4, [r0, console_bcolor]
	ldr r5, [fb_addr]
    call text_putc
    ret
@endf console_erase_cursor

@func console_next_cursor:
    ldri r1, [r0, console_cursor_x]
    add r1, r1, 1
    sub r2, r1, 96
    jlt r2, done

	; We started a new line. Move to next line.
	call console_nextline_cursor
	ret

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_next_cursor

@func console_nextline_cursor:
    mov r1, 0
    ldri r2, [r0, console_cursor_y]
	add r2, r2, 1
    sub r3, r2, 27
    jlt r3, done_all

    strpi [sp, -4], r1
    call console_scroll_up
    ldrip r1, [sp, 4]
	mov r2, 26

done_all:
    stri [r0, console_cursor_y], r2

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_nextline_cursor

@func console_scroll_up:
	; Start of frame buffer.
    ldri r1, [r0, sbuf_start]

	; Skip line.
    add r2, r1, 384

	; End of framebuffer.
    ldri r3, [r0, sbuf_end]

	; Get number of bytes.
    sub r3, r3, r2

	; Convert to words.
    lsr r3, r3, 2

	; Copy back skipping the first line.
    call memcpy32

	; Erase last line.
	ldri r1, [r0, sbuf_end]
	sub r1, r1, 384
	mov r2, 96
	ldri r3, [r0, console_bcolor]
	call memset32

    ret
@endf console_scroll_up

@func console_prev_cursor:
    ldri r1, [r0, console_cursor_x]
    sub r1, r1, 1
    jge r1, done
    mov r1, 95

    ldri r2, [r0, console_cursor_y]
    sub r2, r2, 1
    jlt r2, x_zero
    stri [r0, console_cursor_y], r2
    jmp done

x_zero:
    mov r1, 0

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_prev_cursor


.section data
gvm: .str "GVM Virtual Machine Version 0.01"
gvm_addr: .int gvm
ready: .str "READY."
ready_addr: .int ready
recurring_reg: .int 0x1200410
UI_addr: .int USER_INTERFACE
frame_buffer: .array 10368
fb_addr: .int frame_buffer

.section text

@func KERNEL_MAIN:
    ; Allocate space for console
    sub sp, sp, console_size
    str [console_addr], sp

    ; Initialize console.
    mov r0, sp
    ldr r1,  [fb_addr]
    mov r2, 384    ; 96 x 4 (size of text line in bytes.)
    mul r2, r2, 27 ; 27 (number of lines)
    add r2, r2, r1
    mov r3, 11
    mov r4, 0
    call console_init

    ; Print machine name
    mov r0, sp
    mov r1, 32
    mov r2, 0
    call console_set_cursor

    ldr r6, [gvm_addr]
    call console_puts

    ; Change text color to white.
    mov r0, sp
    mov r1, 15
    mov r2, 0
    call console_set_color

    ; Print ready sign.
    mov r1, 0
    mov r2, 2
    call console_set_cursor
    ldr r6, [ready_addr]
    call console_puts

    ; Print cursor

    mov r0, sp
    mov r1, 0
    mov r2, 3
    call console_set_cursor
    call console_print_cursor

    ; Install our input handler.
    ldr r0, [user_input_handler_addr]
    str [input_jump_addr], r0

    ; Install our display updater.
    ldr r0, [UI_addr]
    str [display_update], r0

    ; Set the recurring timer for 60hz. We will call the display_update
    ; handler that many times.
    mov r0, 60
    ldr r1, [recurring_reg]
    str [r1], r0

    ; Ok, nothing more to do. The recurring timer will take care of updating
    ; everything for us.
loop: wfi
      jmp loop
@endf KERNEL_MAIN

; We wait for a user input and print the value on screen.
@func USER_INTERFACE:
    call USER_INTERFACE_getin
    add r0, r1, 1
    jne r0, process_input
    ret

process_input:
    ; check if we have a control char. If we do, update ui accordingly and get next
    ; input.
    call USER_control_chars
    jeq r0, done
	mov r6, r1

    ; Normal char, print.
    ldr r0, [console_addr]
    call console_putc

    ; Advance cursor.
    ldr r0, [console_addr]
    call console_next_cursor
    call console_print_cursor

done:
    mov r0, 1
    str [should_update], r0
    ret
@endf USER_INTERFACE

; ==== USER_control_chars: checks for control chars and updates
; UI state accordingling.
@func USER_control_chars:
    ; Check for backspace.
    sub r0, r1, 8
    jeq r0, backspace

    ; Check for carriage return.
    sub r0, r1, 13
    jeq r0, enter
    ret

backspace:
    ; Erase cursor at current position.
    ldr r0, [console_addr]
    call console_erase_cursor
    call console_prev_cursor
    call console_print_cursor
    mov r0, 0
    ret

enter:
    ; Erase cursor at current position
    ldr r0, [console_addr]
    call console_erase_cursor
    call console_nextline_cursor
    call console_print_cursor
    mov r0, 0
    ret

@endf USER_control_chars

.section data
wait_input_value: .int 0xFFFFFFFF

.section text
; ===== UI getc. Pools user input.
@func USER_INTERFACE_getin:
    ; r1: returns user input value.

    ; Pool input and return wait_input value if input is not ready.
    ldr r1, [user_input_value]
    add r0, r1, 1
    jeq r0, done

    ; Now set user input to 0 so we don't keep writing stuff over.
    ldr r0, [wait_input_value]
    str [user_input_value], r0
done:
    ret
@endf USER_INTERFACE_getin

.section data
user_input_value: .int 0xFFFFFFFF
user_input_handler_addr: .int USER_input_handler

.section text
; This will be called on every input that is not a quit signal.
USER_input_handler:
    ; The input value will be stored in r0. We just copy it to a user memory
    ; location and return. We will deal with the value later.
    str [user_input_value], r0
    ret



; ============ KERNAL HEAP START. DO NOT CHANGE THIS
.section data
kernel_heap_start: .int 0x0

; ============ KERNAL STACK END. DO NOT CHANGE THIS.
; This is also the start of user code. All gvm programs should start loading
; from 0x100000.
.org 0x100000
.section data
kernel_stack_end: .int 0x0


; ============ Character font.
.org 0x1100000

.embed "./unicode16.chrom"

.section data

; Color table for 256 color pallete.
text_colors:
	.int 0xFF000000 ;   0 - Black
	.int 0xFF000080 ;   1 - Maroon
	.int 0xFF008000 ;   2 - Green
	.int 0xFF008080 ;   3 - Olive
	.int 0xFF800000 ;   4 - Navy
	.int 0xFF800080 ;   5 - Purple
	.int 0xFF808000 ;   6 - Teal
	.int 0xFFC0C0C0 ;   7 - Silver
	.int 0xFF808080 ;   8 - Grey
	.int 0xFF0000FF ;   9 - Red
	.int 0xFF00FF00 ;  10 - Lime
	.int 0xFF00FFFF ;  11 - Yellow
	.int 0xFFFF0000 ;  12 - Blue
	.int 0xFFFF00FF ;  13 - Fuchsia
	.int 0xFFFFFF00 ;  14 - Aqua
	.int 0xFFFFFFFF ;  15 - White
	.int 0xFF000000 ;  16 - Grey0
	.int 0xFF5F0000 ;  17 - NavyBlue
	.int 0xFF870000 ;  18 - DarkBlue
	.int 0xFFAF0000 ;  19 - Blue3
	.int 0xFFD70000 ;  20 - Blue3
	.int 0xFFFF0000 ;  21 - Blue1
	.int 0xFF005F00 ;  22 - DarkGreen
	.int 0xFF5F5F00 ;  23 - DeepSkyBlue4
	.int 0xFF875F00 ;  24 - DeepSkyBlue4
	.int 0xFFAF5F00 ;  25 - DeepSkyBlue4
	.int 0xFFD75F00 ;  26 - DodgerBlue3
	.int 0xFFFF5F00 ;  27 - DodgerBlue2
	.int 0xFF008700 ;  28 - Green4
	.int 0xFF5F8700 ;  29 - SpringGreen4
	.int 0xFF878700 ;  30 - Turquoise4
	.int 0xFFAF8700 ;  31 - DeepSkyBlue3
	.int 0xFFD78700 ;  32 - DeepSkyBlue3
	.int 0xFFFF8700 ;  33 - DodgerBlue1
	.int 0xFF00AF00 ;  34 - Green3
	.int 0xFF5FAF00 ;  35 - SpringGreen3
	.int 0xFF87AF00 ;  36 - DarkCyan
	.int 0xFFAFAF00 ;  37 - LightSeaGreen
	.int 0xFFD7AF00 ;  38 - DeepSkyBlue2
	.int 0xFFFFAF00 ;  39 - DeepSkyBlue1
	.int 0xFF00D700 ;  40 - Green3
	.int 0xFF5FD700 ;  41 - SpringGreen3
	.int 0xFF87D700 ;  42 - SpringGreen2
	.int 0xFFAFD700 ;  43 - Cyan3
	.int 0xFFD7D700 ;  44 - DarkTurquoise
	.int 0xFFFFD700 ;  45 - Turquoise2
	.int 0xFF00FF00 ;  46 - Green1
	.int 0xFF5FFF00 ;  47 - SpringGreen2
	.int 0xFF87FF00 ;  48 - SpringGreen1
	.int 0xFFAFFF00 ;  49 - MediumSpringGreen
	.int 0xFFD7FF00 ;  50 - Cyan2
	.int 0xFFFFFF00 ;  51 - Cyan1
	.int 0xFF00005F ;  52 - DarkRed
	.int 0xFF5F005F ;  53 - DeepPink4
	.int 0xFF87005F ;  54 - Purple4
	.int 0xFFAF005F ;  55 - Purple4
	.int 0xFFD7005F ;  56 - Purple3
	.int 0xFFFF005F ;  57 - BlueViolet
	.int 0xFF005F5F ;  58 - Orange4
	.int 0xFF5F5F5F ;  59 - Grey37
	.int 0xFF875F5F ;  60 - MediumPurple4
	.int 0xFFAF5F5F ;  61 - SlateBlue3
	.int 0xFFD75F5F ;  62 - SlateBlue3
	.int 0xFFFF5F5F ;  63 - RoyalBlue1
	.int 0xFF00875F ;  64 - Chartreuse4
	.int 0xFF5F875F ;  65 - DarkSeaGreen4
	.int 0xFF87875F ;  66 - PaleTurquoise4
	.int 0xFFAF875F ;  67 - SteelBlue
	.int 0xFFD7875F ;  68 - SteelBlue3
	.int 0xFFFF875F ;  69 - CornflowerBlue
	.int 0xFF00AF5F ;  70 - Chartreuse3
	.int 0xFF5FAF5F ;  71 - DarkSeaGreen4
	.int 0xFF87AF5F ;  72 - CadetBlue
	.int 0xFFAFAF5F ;  73 - CadetBlue
	.int 0xFFD7AF5F ;  74 - SkyBlue3
	.int 0xFFFFAF5F ;  75 - SteelBlue1
	.int 0xFF00D75F ;  76 - Chartreuse3
	.int 0xFF5FD75F ;  77 - PaleGreen3
	.int 0xFF87D75F ;  78 - SeaGreen3
	.int 0xFFAFD75F ;  79 - Aquamarine3
	.int 0xFFD7D75F ;  80 - MediumTurquoise
	.int 0xFFFFD75F ;  81 - SteelBlue1
	.int 0xFF00FF5F ;  82 - Chartreuse2
	.int 0xFF5FFF5F ;  83 - SeaGreen2
	.int 0xFF87FF5F ;  84 - SeaGreen1
	.int 0xFFAFFF5F ;  85 - SeaGreen1
	.int 0xFFD7FF5F ;  86 - Aquamarine1
	.int 0xFFFFFF5F ;  87 - DarkSlateGray2
	.int 0xFF000087 ;  88 - DarkRed
	.int 0xFF5F0087 ;  89 - DeepPink4
	.int 0xFF870087 ;  90 - DarkMagenta
	.int 0xFFAF0087 ;  91 - DarkMagenta
	.int 0xFFD70087 ;  92 - DarkViolet
	.int 0xFFFF0087 ;  93 - Purple
	.int 0xFF005F87 ;  94 - Orange4
	.int 0xFF5F5F87 ;  95 - LightPink4
	.int 0xFF875F87 ;  96 - Plum4
	.int 0xFFAF5F87 ;  97 - MediumPurple3
	.int 0xFFD75F87 ;  98 - MediumPurple3
	.int 0xFFFF5F87 ;  99 - SlateBlue1
	.int 0xFF008787 ; 100 - Yellow4
	.int 0xFF5F8787 ; 101 - Wheat4
	.int 0xFF878787 ; 102 - Grey53
	.int 0xFFAF8787 ; 103 - LightSlateGrey
	.int 0xFFD78787 ; 104 - MediumPurple
	.int 0xFFFF8787 ; 105 - LightSlateBlue
	.int 0xFF00AF87 ; 106 - Yellow4
	.int 0xFF5FAF87 ; 107 - DarkOliveGreen3
	.int 0xFF87AF87 ; 108 - DarkSeaGreen
	.int 0xFFAFAF87 ; 109 - LightSkyBlue3
	.int 0xFFD7AF87 ; 110 - LightSkyBlue3
	.int 0xFFFFAF87 ; 111 - SkyBlue2
	.int 0xFF00D787 ; 112 - Chartreuse2
	.int 0xFF5FD787 ; 113 - DarkOliveGreen3
	.int 0xFF87D787 ; 114 - PaleGreen3
	.int 0xFFAFD787 ; 115 - DarkSeaGreen3
	.int 0xFFD7D787 ; 116 - DarkSlateGray3
	.int 0xFFFFD787 ; 117 - SkyBlue1
	.int 0xFF00FF87 ; 118 - Chartreuse1
	.int 0xFF5FFF87 ; 119 - LightGreen
	.int 0xFF87FF87 ; 120 - LightGreen
	.int 0xFFAFFF87 ; 121 - PaleGreen1
	.int 0xFFD7FF87 ; 122 - Aquamarine1
	.int 0xFFFFFF87 ; 123 - DarkSlateGray1
	.int 0xFF0000AF ; 124 - Red3
	.int 0xFF5F00AF ; 125 - DeepPink4
	.int 0xFF8700AF ; 126 - MediumVioletRed
	.int 0xFFAF00AF ; 127 - Magenta3
	.int 0xFFD700AF ; 128 - DarkViolet
	.int 0xFFFF00AF ; 129 - Purple
	.int 0xFF005FAF ; 130 - DarkOrange3
	.int 0xFF5F5FAF ; 131 - IndianRed
	.int 0xFF875FAF ; 132 - HotPink3
	.int 0xFFAF5FAF ; 133 - MediumOrchid3
	.int 0xFFD75FAF ; 134 - MediumOrchid
	.int 0xFFFF5FAF ; 135 - MediumPurple2
	.int 0xFF0087AF ; 136 - DarkGoldenrod
	.int 0xFF5F87AF ; 137 - LightSalmon3
	.int 0xFF8787AF ; 138 - RosyBrown
	.int 0xFFAF87AF ; 139 - Grey63
	.int 0xFFD787AF ; 140 - MediumPurple2
	.int 0xFFFF87AF ; 141 - MediumPurple1
	.int 0xFF00AFAF ; 142 - Gold3
	.int 0xFF5FAFAF ; 143 - DarkKhaki
	.int 0xFF87AFAF ; 144 - NavajoWhite3
	.int 0xFFAFAFAF ; 145 - Grey69
	.int 0xFFD7AFAF ; 146 - LightSteelBlue3
	.int 0xFFFFAFAF ; 147 - LightSteelBlue
	.int 0xFF00D7AF ; 148 - Yellow3
	.int 0xFF5FD7AF ; 149 - DarkOliveGreen3
	.int 0xFF87D7AF ; 150 - DarkSeaGreen3
	.int 0xFFAFD7AF ; 151 - DarkSeaGreen2
	.int 0xFFD7D7AF ; 152 - LightCyan3
	.int 0xFFFFD7AF ; 153 - LightSkyBlue1
	.int 0xFF00FFAF ; 154 - GreenYellow
	.int 0xFF5FFFAF ; 155 - DarkOliveGreen2
	.int 0xFF87FFAF ; 156 - PaleGreen1
	.int 0xFFAFFFAF ; 157 - DarkSeaGreen2
	.int 0xFFD7FFAF ; 158 - DarkSeaGreen1
	.int 0xFFFFFFAF ; 159 - PaleTurquoise1
	.int 0xFF0000D7 ; 160 - Red3
	.int 0xFF5F00D7 ; 161 - DeepPink3
	.int 0xFF8700D7 ; 162 - DeepPink3
	.int 0xFFAF00D7 ; 163 - Magenta3
	.int 0xFFD700D7 ; 164 - Magenta3
	.int 0xFFFF00D7 ; 165 - Magenta2
	.int 0xFF005FD7 ; 166 - DarkOrange3
	.int 0xFF5F5FD7 ; 167 - IndianRed
	.int 0xFF875FD7 ; 168 - HotPink3
	.int 0xFFAF5FD7 ; 169 - HotPink2
	.int 0xFFD75FD7 ; 170 - Orchid
	.int 0xFFFF5FD7 ; 171 - MediumOrchid1
	.int 0xFF0087D7 ; 172 - Orange3
	.int 0xFF5F87D7 ; 173 - LightSalmon3
	.int 0xFF8787D7 ; 174 - LightPink3
	.int 0xFFAF87D7 ; 175 - Pink3
	.int 0xFFD787D7 ; 176 - Plum3
	.int 0xFFFF87D7 ; 177 - Violet
	.int 0xFF00AFD7 ; 178 - Gold3
	.int 0xFF5FAFD7 ; 179 - LightGoldenrod3
	.int 0xFF87AFD7 ; 180 - Tan
	.int 0xFFAFAFD7 ; 181 - MistyRose3
	.int 0xFFD7AFD7 ; 182 - Thistle3
	.int 0xFFFFAFD7 ; 183 - Plum2
	.int 0xFF00D7D7 ; 184 - Yellow3
	.int 0xFF5FD7D7 ; 185 - Khaki3
	.int 0xFF87D7D7 ; 186 - LightGoldenrod2
	.int 0xFFAFD7D7 ; 187 - LightYellow3
	.int 0xFFD7D7D7 ; 188 - Grey84
	.int 0xFFFFD7D7 ; 189 - LightSteelBlue1
	.int 0xFF00FFD7 ; 190 - Yellow2
	.int 0xFF5FFFD7 ; 191 - DarkOliveGreen1
	.int 0xFF87FFD7 ; 192 - DarkOliveGreen1
	.int 0xFFAFFFD7 ; 193 - DarkSeaGreen1
	.int 0xFFD7FFD7 ; 194 - Honeydew2
	.int 0xFFFFFFD7 ; 195 - LightCyan1
	.int 0xFF0000FF ; 196 - Red1
	.int 0xFF5F00FF ; 197 - DeepPink2
	.int 0xFF8700FF ; 198 - DeepPink1
	.int 0xFFAF00FF ; 199 - DeepPink1
	.int 0xFFD700FF ; 200 - Magenta2
	.int 0xFFFF00FF ; 201 - Magenta1
	.int 0xFF005FFF ; 202 - OrangeRed1
	.int 0xFF5F5FFF ; 203 - IndianRed1
	.int 0xFF875FFF ; 204 - IndianRed1
	.int 0xFFAF5FFF ; 205 - HotPink
	.int 0xFFD75FFF ; 206 - HotPink
	.int 0xFFFF5FFF ; 207 - MediumOrchid1
	.int 0xFF0087FF ; 208 - DarkOrange
	.int 0xFF5F87FF ; 209 - Salmon1
	.int 0xFF8787FF ; 210 - LightCoral
	.int 0xFFAF87FF ; 211 - PaleVioletRed1
	.int 0xFFD787FF ; 212 - Orchid2
	.int 0xFFFF87FF ; 213 - Orchid1
	.int 0xFF00AFFF ; 214 - Orange1
	.int 0xFF5FAFFF ; 215 - SandyBrown
	.int 0xFF87AFFF ; 216 - LightSalmon1
	.int 0xFFAFAFFF ; 217 - LightPink1
	.int 0xFFD7AFFF ; 218 - Pink1
	.int 0xFFFFAFFF ; 219 - Plum1
	.int 0xFF00D7FF ; 220 - Gold1
	.int 0xFF5FD7FF ; 221 - LightGoldenrod2
	.int 0xFF87D7FF ; 222 - LightGoldenrod2
	.int 0xFFAFD7FF ; 223 - NavajoWhite1
	.int 0xFFD7D7FF ; 224 - MistyRose1
	.int 0xFFFFD7FF ; 225 - Thistle1
	.int 0xFF00FFFF ; 226 - Yellow1
	.int 0xFF5FFFFF ; 227 - LightGoldenrod1
	.int 0xFF87FFFF ; 228 - Khaki1
	.int 0xFFAFFFFF ; 229 - Wheat1
	.int 0xFFD7FFFF ; 230 - Cornsilk1
	.int 0xFFFFFFFF ; 231 - Grey100
	.int 0xFF080808 ; 232 - Grey3
	.int 0xFF121212 ; 233 - Grey7
	.int 0xFF1C1C1C ; 234 - Grey11
	.int 0xFF262626 ; 235 - Grey15
	.int 0xFF303030 ; 236 - Grey19
	.int 0xFF3A3A3A ; 237 - Grey23
	.int 0xFF444444 ; 238 - Grey27
	.int 0xFF4E4E4E ; 239 - Grey30
	.int 0xFF585858 ; 240 - Grey35
	.int 0xFF626262 ; 241 - Grey39
	.int 0xFF6C6C6C ; 242 - Grey42
	.int 0xFF767676 ; 243 - Grey46
	.int 0xFF808080 ; 244 - Grey50
	.int 0xFF8A8A8A ; 245 - Grey54
	.int 0xFF949494 ; 246 - Grey58
	.int 0xFF9E9E9E ; 247 - Grey62
	.int 0xFFA8A8A8 ; 248 - Grey66
	.int 0xFFB2B2B2 ; 249 - Grey70
	.int 0xFFBCBCBC ; 250 - Grey74
	.int 0xFFC6C6C6 ; 251 - Grey78
	.int 0xFFD0D0D0 ; 252 - Grey82
	.int 0xFFDADADA ; 253 - Grey85
	.int 0xFFE4E4E4 ; 254 - Grey89
	.int 0xFFEEEEEE ; 255 - Grey93


