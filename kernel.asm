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
    ; Reset the jiffy counter and jump to user code.
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
	strpi [sp, -4], r0
	strpi [sp, -4], r1

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
	ldrip r1, [sp, 4]
	ldrip r0, [sp, 4]
	ret

quit:
	; Quit means we want to turn of the cpu.
	halt
@endf input_handler

.section data
display_update: .int 0x0

.section text
@func recurring_handler:
	call flush_video
	strpi [sp, -4], r0
	ldr r0, [display_update]
	jeq r0, flush
	call r0

flush:
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
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	sub r3, r3, 1
	jgt r3, memcpy
	ret

; ==== Memcopy16. Same as memcpy but assumes size is a multiple of 16 words.
memcpy16:
	; r1: start to-address
	; r2: start from:address
	; r3: size in words.
	; r4: local variable for copying memory.
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	ldrip r4, [r2, 4]
	strip [r1, 4], r4
	sub r3, r3, 16
	jgt r3, memcpy16
	ret

; ==== HLine: draws a horizontal line on the screen.
; Linelength is 2560 bytes (640 * 32bpp)
@func hline:
	; r1: y-pos
	; r2: x-start
	; r3: y-start
	; r4: width
	; r5: color (RGBA)

	; Multiply y-pos by 2560 to get y in the frameuffer.
	mul r1, r1, 2560

	; Multiply x-start and x-end by 4 for pixel size.
	lsl r3, r3, 2

width:
	lsl r8, r2, 2

	; Now add mem start, x-start with y-pos to get the framebuffer start point.
	add r7, r1, r8
	ldr r9, [0x84]
	add r7, r7, r9

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
	add r1, r1, 2560

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

	; Multiply x-pos by 4 to get x in the framebuffer.
	lsl r1, r1, 2

	; Multiply y-start and y-end by 2560 to get their positions.
	mul r3, r3, 2560

width:
	mul r8, r2, 2560

	; Now add mem start, x-pos, y-start with y-end to get the framebuffer start point.
	add r7, r1, r8
	ldr r9, [0x84]
	add r7, r7, r9

line:
	; Write the pixel at the location.
	strip [r7, 2560], r5

	; Increment y-start.
	add r8, r8, 2560

	; Check if we got to y-end
	sub r6, r3, r8

	; If line is not done, loop.
	jne r6, line

	; Line is not done. Need to subtract a line from framebuffer because we
	; optimistically assume we need to increment.
	sub r7, r7, 2560

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
    ldr r1, [r20]
    and r1, r1, 0xFFFF
    jeq r1, done

    ; Save context in stack before calling putc.
    strpi [sp, -4], r2
    strpi [sp, -4], r3
    strpi [sp, -4], r4
    strpi [sp, -4], r5
    call putc

    ; Restore context.
    ldrip r5, [sp, 4]
    ldrip r4, [sp, 4]
    ldrip r3, [sp, 4]
    ldrip r2, [sp, 4]

    ; Update (x,y)
    call incxy

    ; Next char in same word
    ldr r1, [r20]
    lsr r1, r1, 16
    jeq r1, done

    strpi [sp, -4], r2
    strpi [sp, -4], r3
    strpi [sp, -4], r4
    strpi [sp, -4], r5
    call putc
    ldrip r5, [sp, 4]
    ldrip r4, [sp, 4]
    ldrip r3, [sp, 4]
    ldrip r2, [sp, 4]

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
; ==== PutC: Prints a character on the screen.
@func putc:
	; r1: Character unicode value
	; r2: x-pos
	; r3: y-pos
	; r4: foreground color
	; r5: background color

	; To find the (x,y) position in the frame buffer, we use the formula
	; pos(x,y) = x-pos*8*4 + 0x84 + y-pos * lineLength * 16.
	ldr r6, [0x84]
	mul r2, r2, 32
	add r2, r2, r6
	mul r3, r3, 2560
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

	; Number of rows per character.
	mov r6, 4

reset_pixel_word_counter:
	; Load the character word
	ldr r1, [r3]

	; Number of pixels per word.
	mov r8, 32

reset_pixel_row_counter:
	; Number of pixels per row.
	mov r7, 8

main_loop:
	; For each 8 pixel row, check if we need to write the fore or background
	; color.
	and r0, r1, 1
	jeq r0, background_color

	; Foreground.
	strip [r2, -4], r4
	jmp next_pixel

background_color:
	strip [r2, -4], r5

next_pixel:
	; Shift the pixel row.
	lsr r1, r1, 1

	; Check if row is done.
	sub r7, r7, 1

    ; If not then loop back.
	jne r7, main_loop

	; Reposition the frame buffer on the next row.
	add r2, r2, 2592  ; 32 + 2560.

	; Now check if all pixels in word are done.
	sub r8, r8, 8

	; If all pixels from row are not done, loop back.
	jne r8, reset_pixel_row_counter

	; All pixels in word are done. Check if we are done.
	sub r6, r6, 1

	; Get the next word row and loop.
	add r3, r3, 4
	jne r6, reset_pixel_word_counter

	ret
@endf putc

; ==== Fill816: Fills an 8x16 pixels block with the same value.
@func fill816:
	; r1: x-pos
	; r2: y-pos
	; r3: value to set.

	; To find the (x,y) position in the frame buffer, we use the formula
	; pos(x,y) = x-pos*8*4 + 0x84 + y-pos * lineLength * 16.
	ldr r6, [0x84]
	mul r1, r1, 32
	add r1, r1, r6
	mul r2, r2, 2560
	lsl r2, r2, 4
	add r1, r1, r2

	; Set the row counter to 16.
	mov r2, 16

loop:
	; Each pixel is 4 bytes long, so we need to write 32 bytes per row.
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	strip [r1, 4], r3
	add r1, r1, 2528

	; Check if we are done.
	sub r2, r2, 1
	jne r2, loop

	; We are done.
	ret
@endf fill816

; ==== FlushVideo: tells the video controller that it can copy the framebuffer
; to its own memory.
@func flush_video:
	ldr r0, [0x80]
	mov r1, 1
    str [r0], r1
	ret
@endf flush_video

; ==== WaitVideo: waits for the video controller to signal that the framebuffer
; available for writing.
@func wait_video:
	ldr r0, [0x80]
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

	; Set the screen to update on 30Hz.
	mov r0, 30
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

	; check if we have a control char. If we do, update ui accordingly and get next
	; input.
	call USER_control_chars
	jeq r0, done


	; Set the params for putc.
	ldr r2, [ui_x]
	ldr r3, [ui_y]
	ldr r4, [ui_fcolor]
	ldr r5, [ui_bcolor]

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

	call fill816
	call flush_video

	mov r0, 0
	ret
@endf USER_control_chars

.section data
wait_input_value: .int 0xFFFFFFFF

.section text
; ===== UI getc. Waits for user input.
USER_INTERFACE_getin:
	; r1: returns user input value.

	; Wait until user input != 0 or 1 second has passed.
	wfi
	ldr r1, [user_input_value]
	add r0, r1, 1
	jeq r0, USER_INTERFACE_getin

	; Now set user input to 0 so we don't keep writing stuff over.
	ldr r0, [wait_input_value]
	str [user_input_value], r0
	ret

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
