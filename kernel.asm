; GVM Kernel
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
	jmp reset_handler	  ; Reset
	ret					  ; Oneshot timer handler
	jmp input_handler	  ; Input handler
	jmp recurring_handler ; Recurring timer handler

.org 0x80
.section data
vram_reg:   .int 0x1200000
vram_start: .int 0x101F000

.section text

; ==== Reset interrupt handler.
reset_handler:
	mov r0, 0

	; Clear input register
	ldr r1, [input_value_addr]
	str [r1], r0

	; Clear user input vector address.
	str [input_jump_addr], r0

	; Now jump to main kernel code.
	jmp MAIN

.section data
input_value_addr: .int 0x1200004
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

	; Ok, need to update. But first, update should_update.
	mov r0, 0
	str [should_update], r0

	ldr r1, [vram_start]
	ldr r2, [fb_addr]
	ldr r3, [fb_size_words]

	call memcpy32
	call flush_video

done:
	ldpip r3, r4, [sp, 8]
	ldpip r1, r2, [sp, 8]
	ldrip r0, [sp, 4]
	ret
@endf recurring_handler

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
  .equ kLineLength 2560

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

text_colors_addr: .int text_colors

.section text

; ==== PutS: Prints a string on the screen.
@func puts:
    ; r1: Address to string start.
    ; r2: x-pos start.
    ; r3: y-pos start.
    ; r4: foreground color.
    ; r5: background color.

    ; We copy the start addres to r1 because we will use r1 as the actual
    ; char value to print with putc.
    mov r20, r1

loop:
    ; Chars in string are 16-bit wide. So we need to AND and shift.
    ldr r22, [r20]
    and r1, r22, 0xFFFF
    jeq r1, done

    ; Save context in stack before calling putc.
    stppi [sp, -8], r2, r3
    stppi [sp, -8], r4, r5
	ldr r6, [fb_addr]

	call putc

    ; Restore context.
    ldpip r4, r5, [sp, 8]
    ldpip r2, r3, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Next char in same word
    lsr r1, r22, 16
    jeq r1, done

    stppi [sp, -8], r2, r3
    stppi [sp, -8], r4, r5
	ldr r6, [fb_addr]

    call putc

    ldpip r4, r5, [sp, 8]
    ldpip r2, r3, [sp, 8]

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
    ; r2: x-pos
    ; r3: y-pos
    add r2, r2, 1
    sub r21, r2, 80
    jne r21, done

    mov r2, 0
    add r3, r3, 1

done:
    ret
@endf incxy

.section data
chrom_addr: .int 0x1100000

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

	; Translate colors 0-15 to their RGBA values by multiplying the value by 4
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
	ldr r0, [vram_reg]
	mov r1, 1
    str [r0], r1
	ret
@endf flush_video

; ==== WaitVideo: waits for the video controller to signal that the framebuffer
; available for writing.
@func wait_video:
	ldr r0, [vram_reg]
	ldr r0, [r0]
	jne r0, wait_video
	ret
@endf wait_video

; ======================== User interface code =======================

.section data

ui_x: .int 0
ui_y: .int 0
ui_fcolor: .int 15
ui_bcolor: .int 0

ready: .str "READY"
ready_addr: .int ready
recurring_reg: .int 0x1200010
UI_addr: .int USER_INTERFACE
frame_buffer: .array 921600
fb_addr: .int frame_buffer

.section text

@func MAIN:
    ; Print ready sign.
    ldr r1, [ready_addr]
    mov r2, 0
    mov r3, 0
    ldr r4, [ui_fcolor]
    ldr r5, [ui_bcolor]
    call puts
    mov r0, 1
    str [ui_y], r0

	; Install our input handler.
	ldr r0, [user_input_handler_addr]
	str [input_jump_addr], r0

	; Install our display updater.
    ldr r0, [UI_addr]
    str [display_update], r0

	; Set the recurring timer for 120hz. We will call the display_update
	; handler that many times.
	mov r0, 120
	ldr r1, [recurring_reg]
	str [r1], r0

	; Ok, nothing more to do. The recurring timer will take care of updating
	; everything for us.
loop: wfi
	  jmp loop
@endf MAIN

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


	; Set the params for putc.
	ldr r2, [ui_x]
	ldr r3, [ui_y]
	ldr r4, [ui_fcolor]
	ldr r5, [ui_bcolor]
	ldr r6, [fb_addr]

	call putc

	; Update x position
	ldr r2, [ui_x]
	add r2, r2, 1
	sub r4, r2, 80
	jeq r4, x_end
	str [ui_x], r2
	jmp done

x_end:
	; We reached the end of the screen. Wrap back.
	mov r2, 0

	ldr r3, [ui_y]
	add r3, r3, 1
	str [ui_x], r2
	str [ui_y], r3

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
	ret

backspace:
	; Load the x pos and decrease it.
	ldr r1, [ui_x]
	sub r1, r1, 1

	; if it's >= 0, we can erase the block.
	jge r1, backspace_erase

	; if < 0 then we need to load update ui_y
	ldr r2, [ui_y]
	sub r2, r2, 1

	; if >= 0 then set ui_x and ui_y to 0.
	jge r2, backspace_move_x_to_end
	mov r1, 0
	mov r2, 0
	str [ui_y], r2
	jmp backspace_erase

backspace_move_x_to_end:
	; if >= 0 then move ui_x to 79.
	mov r1, 79
	str [ui_y], r2

backspace_erase:
	str [ui_x], r1
	ldr r2, [ui_y]
	ldr r3, [ui_bcolor]

	ldr r0, [text_colors_addr]
	lsl r3, r3, 2
	ldri r3, [r0, r3]
	ldr r4, [fb_addr]

	call fill816

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


.org 0x1100000

.embed "./unicode16.chrom"
