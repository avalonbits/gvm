; GVM Kernel
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
	jmp reset_handler
	jmp timer_handler
	jmp input_handler

.org 0xE108C
.section text

; ==== Reset interrupt handler.
reset_handler:
    ; Reset the jiffy counter and jump to user code.
	mov r0, 0

    ; 64 bit counter words are 0xE1084 (LSW) and 0xE1088 (MSW)
	str [0xE1084], r0
	str [0xE1088], r0

	; Now jump to user code, which starts at the 1MiB address.
	jmp USER_CODE

; ==== Timer interrupt handler.
timer_handler:
	; Implements a 64 bit jiffy counter.
	; Save the contents of r0 on the stack so we don't disrupt user code.
	sub r30, r30, 4
	str [r30], r0

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
	ldr r0, [r30]
	add r30, r30, 4
	ret

; ==== Input handler
input_handler:
	; Save the contents of r0 on the stack so we don't disrupt user code.
	sub r30, r30, 4
	str [r30], r0

	; Read the value from the input.
	ldr r0, [0xE10D4]

	; Quit is the value 0xFFFFFFFF so adding 1 should result in 0.
	add r0, r0, 1
	jeq r0, input_handler_quit

	; Input processing done. Restore restore r0 and return.
	ldr r0, [r30]
	add r30, r30, 4
	ret

input_handler_quit:
	; Quit means we want to turn of the cpu.
	halt

.section data
	.int kLineLength 2560  ; 640 * 4 (32bpp)

.section text
; ==== HLine: draws a horizontal line on the screen.
hline:
	; r1: y-pos
	; r2: x-start
	; r3: y-start
	; r4: width
	; r5: color (RGBA)

	; Multiply y-pos by kLineLength to get y in the frameuffer.
	mul r1, r1, kLineLength

	; Multiply x-start and x-end by 4 for pixel size.
	lsl r3, r3, 2

hline_width:
	lsl r8, r2, 2

	; Now add mem start, x-start with y-pos to get the framebuffer start point.
	add r7, r1, r8
	add r7, r7, 0x84

hline_line:
	; Write pixel to framebuffer location
	str [r7], r5

	; Increment x-start by pixel size
	add r8, r8, 4

	; Check if we got to x-end.
	sub r6, r3, r8
	jeq r6, hline_line_end

	; Line is not done. Increment framebuffer and loop.
	add r7, r7, 4
	jmp hline_line

hline_line_end:
	; Down with one line.
	sub r4, r4, 1
	jeq r4, hline_line_done

	; Line is still wide.
	add r1, r1, kLineLength
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

	; Multiply y-start and y-end by kLineLength to get their positions.
	mul r3, r3, kLineLength

vline_width:
	mul r8, r2, kLineLength

	; Now add mem start, x-pos, y-start with y-end to get the framebuffer start point.
	add r7, r1, r8
	add r7, r7, 0x84

vline_line:
	; Write the pixel at the location.
	str [r7], r5

	; Increment y-start.
	add r8, r8, kLineLength

	; Check if we got to y-end
	sub r6, r3 ,r8
	jeq r6, vline_line_end

	; Line is not done. Increment framebuffer and loop.
	add r7, r7, kLineLength

vline_line_end:
	; Done with one line.
	sub r4, r4, 1
	jeq r4,  vline_done

	; Line is still wide.
	add r1, r1, 4
	jmp vline_width

vline_done:
	ret

.org 0x100000

.section data

border_color: .int kRED 0xFF0000FF

.section text

USER_CODE:
	; Draws a border around the screen. Then waits for 5 seconds and halts.

	; Load the current tick value to r0.
	ldr r0, [0xE1084]

	; Now, set r5 to the border color.
	ldr r5, [border_color]
