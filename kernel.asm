; GVM Kernel
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
	jmp reset_handler
	jmp timer_handler
	jmp input_handler

.org 0xE1094

.embed "./latin1.chrom"

.section text

; ==== Reset interrupt handler.
reset_handler:
    ; Reset the jiffy counter and jump to user code.
	mov r0, 0

    ; 64 bit counter words are 0xE1084 (LSW) and 0xE1088 (MSW)
	str [0xE1084], r0
	str [0xE1088], r0

	; Clear input register
	str [0xE108C], r0

	; Clear user input vector address.
	str [0xE1090], r0

	; Now jump to user code, which starts at the 1MiB address.
	jmp USER_CODE


; ==== Timer interrupt handler.
timer_handler:
	; Implements a 64 bit jiffy counter.
	; Save the contents of r0 on the stack so we don't disrupt user code.
	strpi [r30, -4], r0

	; Now increment the LSB of the 64 counter.
	ldr r0, [0xE1084]  ; Load the LSW to r0.
    add r0, r0, 1      ; increment it.
	str [0xE1084], r0  ; write it back.

	; If r0 != 0 that means we did not overflow.
	jne r0, timer_handler_done

	; Otherwise, we need to increment the MSW.
	ldr r0, [0xE1088]  ; Loadthe MSW to r0.
	add r0, r0, 1	   ; increment it.
    str [0xE1088], r0  ; write it back.

timer_handler_done:
	; Now lets restore r0 and return.
	ldrip r0, [r30, 4]
	ret


; ==== Input handler
input_handler:
	; Save the contents of r0 and r1 on the stack so we don't disrupt user code.
	strpi [r30, -4], r0
	strpi [r30, -4], r1

	; Read the value from the input.
	ldr r0, [0xE108C]

	; Quit is the value 0xFFFFFFFF so adding 1 should result in 0.
	add r1, r0, 1
	jeq r1, input_handler_quit

	; Load the user jump address. If it's != 0, call it.
	ldr r1, [0xE1090]
	jeq r1, input_handler_done
	call r1

input_handler_done:
	; Input processing done. Restore restore r1 and r0 and return.
	ldrip r1, [r30, 4]
	ldrip r0, [r30, 4]
	ret

input_handler_quit:
	; Quit means we want to turn of the cpu.
	halt


; ==== Memset. Sets a memory region to a specific value.
memset:
	; r1: start address
	; r2: size in words
	; r3: value to set.
	strip [r1, 4], r3
	sub r2, r2, 1
	jgt r2, memset
	ret

; ==== Memset16. Same as memset but assumes size is a multiple of 16 words.
memset16:
	; r1: start address
	; r2: size in words. MUST BE A MULTIPLE OF 16.
	; r3: value to set.
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
    strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
    strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
    strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
    strip [r1, 4], r3
	sub r2, r2, 16
	jgt r2, memset16
	ret

; ==== Memcopy. Copies the contents of one region of memory to another.
; Does not handle overlap.
memcpy:
	; r1: start to-address
	; r2: start from:address
	; r3: size in words.
	; r4: local variable for copying memory.
	ldr r4, [r2]
	strip [r1, 4], r4
	add r2, r2, 4
	sub r3, r3, 1
	jgt r3, memcpy
	ret

; ==== HLine: draws a horizontal line on the screen.
; Linelength is 2560 bytes (640 * 32bpp)
hline:
	; r1: y-pos
	; r2: x-start
	; r3: y-start
	; r4: width
	; r5: color (RGBA)

	; Multiply y-pos by 2560 to get y in the frameuffer.
	mul r1, r1, 2560

	; Multiply x-start and x-end by 4 for pixel size.
	lsl r3, r3, 2

hline_width:
	lsl r8, r2, 2

	; Now add mem start, x-start with y-pos to get the framebuffer start point.
	add r7, r1, r8
	add r7, r7, 0x84

hline_line:
	; Write pixel to framebuffer location
	strip [r7, 4], r5

	; Increment x-start by pixel size
	add r8, r8, 4

	; Check if we got to x-end.
	sub r6, r3, r8

	; If not, loop back and continue.
	jne r6, hline_line

	; Finished line. Need to subtract one from framebuffer because we
    ; optimistically assume we need to increment.
	sub r7, r7, 1

	; Down with one line.
	sub r4, r4, 1
	jeq r4, hline_line_done

	; Line is still wide.
	add r1, r1, 2560
	jmp hline_width

hline_line_done:
	ret

; ==== VLine: draws a vertical line on the screen.
vline:
	; r1: x-pos
	; r2: y-start
	; r3: y-end
	; r4: width
	; r5: color (RGBA)

	; Multiply x-pos by 4 to get x in the framebuffer.
	lsl r1, r1, 2

	; Multiply y-start and y-end by 2560 to get their positions.
	mul r3, r3, 2560

vline_width:
	mul r8, r2, 2560

	; Now add mem start, x-pos, y-start with y-end to get the framebuffer start point.
	add r7, r1, r8
	add r7, r7, 0x84

vline_line:
	; Write the pixel at the location.
	strip [r7, 2560], r5

	; Increment y-start.
	add r8, r8, 2560

	; Check if we got to y-end
	sub r6, r3, r8

	; If line is not done, loop.
	jne r6, vline_line

	; Line is not done. Need to subtract a line from framebuffer because we
	; optimistically assume we need to increment.
	sub r7, r7, 2560

	; Done with one line.
	sub r4, r4, 1
	jeq r4,  vline_done

	; Line is still wide.
	add r1, r1, 4
	jmp vline_width

vline_done:
	ret

; ====== Text mode functions and data.

.section data

; Text mode supports 16 colors and all functions require a foreground and a
; background color.
text_colors:
	.int 0xFF000000 ;  0 - Black
	.int 0xFF000080 ;  1 - Maroon
	.int 0xFF008000 ;  2 - Green
	.int 0xFF008080 ;  3 - Olive
	.int 0xFF800000 ;  4 - Navy
	.int 0xFF800080 ;  5 - Purple
	.int 0xFF808000 ;  6 - Teal
	.int 0xFFC0C0C0 ;  7 - Silver
	.int 0xFF808080 ;  8 - Grey
	.int 0xFF0000FF ;  9 - Read
	.int 0xFF00FF00 ; 10 - Lime
	.int 0xFF00FFFF ; 11 - Yellow
	.int 0xFFFF0000 ; 12 - Blue
	.int 0xFFFF00FF ; 13 - Fuchsia
	.int 0xFFFFFF00 ; 14 - Aqua
	.int 0xFFFFFFFF ; 15 - White

text_colors_addr:
	.int text_colors

.section text
; ==== PutC: Prints a character on the screen.
putc:
	; r1: Character unicode value
	; r2: x-pos
	; r3: y-pos
	; r4: foreground color
	; r5: background color

	; To find the (x,y) position in the frame buffer, we use the formula
	; pos(x,y) = x-pos*8*4 + 0x84 + y-pos * lineLength * 16.
	mul r2, r2, 32
	add r2, r2, 0x84
	mul r3, r3, 2560
	lsl r3, r3, 4
	add r2, r2, r3

	; And because we process the row from right to left, we need to move the
	; start 8 pixels (3 bytes) to the right. But because this is 0 based, we
	; need to add 31.
	add r2, r2, 31

	; Translate colors 0-15 to their RGBA values by multiplying the value by 4
	; and then summing it with the start of the color table.
	ldr r0, [text_colors_addr]
	lsl r4, r4, 2
	add r4, r0, r4
	ldr r4, [r4]
	lsl r5, r5, 2
	add r5, r0, r5
	ldr r5, [r5]

	; Each character is 8x16 pixels encoded in 16 bytes with each byte being an
	; 8 pixel row. In order to find the start of the char we multiply the char
	; by 16 and sum it with the start of the character rom.
	mov r0, 0xE1094
	lsl r1, r1, 4
	add r1, r0, r1

	; Copy of character start.
	mov r3, r1

	; Number of rows per character.
	mov r6, 4

putc_reset_pixel_word_counter:
	; Load the character word
	ldr r1, [r3]

	; Number of pixels per word.
	mov r8, 32

putc_reset_pixel_row_counter:
	; Number of pixels per row.
	mov r7, 8

putc_main_loop:
	; For each 8 pixel row, check if we need to write the fore or background
	; color.
	and r0, r1, 1
	jeq r0, putc_background_color

	; Foreground.
	strip [r2, -4], r4
	jmp putc_next_pixel

putc_background_color:
	strip [r2, -4], r5

putc_next_pixel:
	; Shift the pixel row.
	lsr r1, r1, 1

	; Check if row is done.
	sub r7, r7, 1
	jeq r7, putc_row_done

	jmp putc_main_loop

putc_row_done:
	; Reposition the frame buffer on the next row.
	add r2, r2, 2592  ; 32 + 2560.

	; Now check if all pixels in word are done.
	sub r8, r8, 8

	; If all pixels from row are not done, loop back.
	jne r8, putc_reset_pixel_row_counter

	; All pixels in word are done. Check if we are done.
	sub r6, r6, 1
	jeq r6, putc_done

	; Not done yet. Get the next word row and loop.
	add r3, r3, 4
	jmp putc_reset_pixel_word_counter

putc_done:
	ret

.org 0x100000

.section data

border_color: .int 0xFF0000FF
back_color:   .int 0xFF0084A1
screen_size:  .int 230400     ; 640 * 360 words.
wait_input_value: .int 0xFFFFFFFF

.section text

; We wait for a user input and print the value on screen.
USER_CODE:
	; Install our input handler.
	ldr r0, [user_input_handler_addr]
	str [0xE1090], r0

	; Now set the x,y start values.
	mov r2, 0
	mov r3, 0

USER_CODE_wait_input:
	; Wait until user input != 0
	ldr r1, [user_input_value]
	add r0, r1, 1
	jeq r0, USER_CODE_wait_input

	; Now set user input to 0 so we don't keep writing stuff over.
	ldr r0, [wait_input_value]
	str [user_input_value], r0

USER_CODE_wait_video:
	; Wait for video memory to be available.
	ldr r0, [0x80]
	jne r0, USER_CODE_wait_video

	; Now we have r1, r2 and r3 correctly set.
	; Copy r2 and r3 to stack before calling PutC.
	strpi [r30, -4], r2
	strpi [r30, -4], r3
	mov r4, 15
	mov r5, 0

	call putc

	; Signal video controller.
	mov r0, 1
	str [0x80], r0

	; Copy r3 and r2 back from stack.
	ldrip r3, [r30, 4]
	ldrip r2, [r30, 4]
	add r2, r2, 1

	; Ok, character written. Loop back and wait more.
	jmp USER_CODE_wait_input


USER_CODE_done:
	halt

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

