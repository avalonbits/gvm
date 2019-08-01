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

.include "./includes/memory.asm" as memory
.include "./includes/strings.asm" as strings
.include "./includes/textmode.asm" as textmode

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt. There are 256
; possible interrupts. If no handler is available, we use a RET instruction so that the
; interrupt is ignored. The reset handler takes care of setting the other RET instructions
; to avoid writing a huge list of values.
interrupt_table:
    jmp reset_handler
	ret
	jmp input_handler
	jmp recurring_handler

.org 0x400
.section data
; ===== Kernel function table.
register_interrupt: .int _register_interrupt
get_interrupt:      .int _get_interrupt
malloc:             .int _malloc
free:               .int _free
getkey:             .int _getkey
getc:				.int _getc
text_putc:          .int textmode.putc
putc:				.int _putc
puts:               .int _puts
strlen:             .int strings.length
memcpy:             .int memory.copy
memcpy2:            .int memory.copy2
memcpy32:           .int memory.copy32
memset:             .int memory.set
memset2:            .int memory.set2
memset32:           .int memory.set32
itoa:               .int strings.itoa

.org 0x2400
.section data
vram_reg:   .int 0x1200400
vram_start: .int 0x1000000
ptr_heap_start: .int heap_start

.equ JMP 0x15  ; jmp instruction for registering handler.
.equ RET 0x1e  ; ret instruction for clearing interrupt vector.

.section text

; ==== RegisterInterrrupt. Registers a function as an interrupt handler.
@func _register_interrupt:
	; r0: Returns 0 on failure, 1 otherise.
	; r1: interrupt value.
	; r2: absolute function address to call on interrupt. Must use <= 26 bits.

	; Interrupt values range from 0-255.
	jlt r1, invalid_interrupt
	sub r0, r1, 0xFF
	jgt r0, invalid_interrupt

	; We have a valid interrupt. Write the function pointer to the
	; interrupt vector.
	lsl r1, r1, 2    ; Each value in the vector is 4 bytes long.
	sub r2, r2, r1   ; We subtract the vector value because jmp is pc relative.
	lsl r2, r2, 6	 ; Make space for jump instruction
	orr r2, r2, JMP  ; jmp instruction
	str [r1], r2
	mov r0, 1
	ret

invalid_interrupt:
	mov r0, 0
	ret
@endf _register_interrupt

; ==== GetInterrrupt. Returns the function address registered for interrupt.
@func _get_interrupt:
	; r0: Returns 0 on failure, != 0 otherise.
	; r1: interrupt value.

	; Interrupt values range from 0-255.
	jlt r1, invalid_interrupt
	sub r0, r1, 0xFF
	jgt r0, invalid_interrupt

	; We have a valid interrupt. Get the pc relative funcrtion addreess, make
	; it absolute and return.
	lsl r1, r1, 2  ; Each value in the vector is 4 bytes long.
	ldr r0, [r1]
	lsr r0, r0, 6  ; Get rid of the jump instruction.
	add r0, r0, r1 ; Add the vector index offset to get the absolute adress
	ret

invalid_interrupt:
	mov r0, 0
	ret
@endf _get_interrupt

; ==== Reset interrupt handler.
reset_handler:
	; Set the interrupt vector starting from the 5th interrupt with RET instruction.
	mov r1, 0x10
	mov r2, 0xFC
	mov r3, RET
	call memory.set4

	; We initialize the first two words of the heap to zero. This corresponds
	; to the header fields size and next.
	ldr r0, [ptr_heap_start]
	stpip [r0, 0x0], rZ, rZ

    ; Clear input register
    ldr r1, [input_value_addr]
    str [r1], rZ

	; Reset input buffer
	mov r0, input_buffer
	stri [r0, ib_head], rZ
	stri [r0, ib_tail], rZ

    ; Now jump to main kernel code.
    jmp KERNEL_MAIN

.section data
input_value_addr: .int 0x1200404
	; struct input_buffer. Defines 32 key long circular buffer.
	.equ ib_buffer       0
	.equ ib_head        128  ; 32 * 4.
	.equ ib_tail        132
	.equ ib_struct_size 136
input_buffer: .array 136

.section text
; ==== Input handler
; The input handler will only add new key inputs to the circular buffer. If the
; buffer is full, the input buffer is overwritten
@func input_handler:
    ; Save the contents of r0 and r2 on the stack so we don't disrupt user code.
    stppi [sp, -8], r0, r2

    ; Read the value from the input.
    ldr r2, [input_value_addr]
    ldr r2, [r2]

    ; Quit is the value 0xFFFFFFFF so adding 1 should result in 0.
    add r0, r2, 1
    jeq r0, quit

	; Save r1 and r3 to the stack.
	stppi [sp, -8], r1, r3

	; Load input buffer.
	mov r0, input_buffer

    ; Load the buffer tail and write.
	ldri r1, [r0, ib_tail]

	; Because we operate with words, we need to multiply r1 by 4.
	lsl r3, r1, 2
	add r3, r0, r3
	str [r3], r2

	; Update the tail
	add r1, r1, 1

	; If it is not 32, then just write it back and be done.
	sub r3, r1, 32
	jne r3, done

	; Need to wrap around.
	mov r1, 0

done:
	stri [r0, ib_tail], r1

    ; Input processing done. Restore registers and return.
	ldpip r1, r3, [sp, 8]
    ldpip r0, r2, [sp, 8]
    ret

quit:
    ; Quit means we want to turn of the cpu.
    halt
@endf input_handler

; === GetKey. Reads the next key from the key input buffer.
@func _getkey:
	; r0: returns the key read. r0 == 0 means no input available.

	; The first thing to do is to get ib_tail. This way, if an interrupt
    ; happens while we are in this function, we will reduce the chance of
	; buffer overwrite.
	mov r0, input_buffer
	ldri r2, [r0, ib_tail]
	ldri r1, [r0, ib_head]

	;  If tail != head then the buffer has keys so we can read it.
	sub r2, r2, r1
	jne r2, read_input

	; tail == head means no key in buffer. Return 0.
	mov r0, 0
	ret

read_input:
	; We read the input to r2, update ib_head then move the result to r0 to
	; return it to caller.

	; Because we operate on words, we need to multiply ib_head by 4.
	lsl r2, r1, 2

	; Now get the char at buffer[r2]
	add r2, r0, r2
	ldr r2, [r2]

	; Update ib_head and wrap if needed.
	add r1, r1, 1
	sub r3, r1, 32
	jne r3, done

	; We need to wrap back to 0.
	mov r1, rZ

done:
	stri [r0, ib_head], r1
	mov r0, r2
	ret
@endf _getkey

; ==== GetC: returns the character pressed. Ignores control keys and
; keyup events.
@func _getc:
	; r0: Returns the character. Returns 0 in case no character is available,
	ldr r0, [getkey]
	call r0
	jeq r0, done

	; Check if this is a keyup event. If it is, ignore it.
	lsr r1, r0, 31
	and r1, r1, 1
	jne r1, return_no_key

	; Check if this is a control key. If it is, ignore it.
	lsr r1, r0, 30
	and r1, r1, 1
	jne r1, return_no_key

	; We got a charater code. Just return it.
	jmp done

return_no_key:
	mov r0, 0
done:
	ret
@endf _getc

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

; ==== Allocator: malloc and free.
.section data
heap_lower_limit: .int heap_start
heap_curr_limit: .int heap_start
ptr_heap_curr_limit: .int heap_curr_limit

    ; struct memory_header
    .equ mh_bytes 0
	.equ mh_next  4
	.equ mh_size  8

	.equ memory_page_shift 4  ; Shifting by 4 bits gives 16 bytes per page.

.section text
@func _malloc:
	; r0 returns address of memory. If < 0 no memory was available.
	; r1: Size in bytes to allocate.

	ldr r2, [ptr_heap_curr_limit]
	ldr r3, [heap_lower_limit]

	; We subtract 8 from sp to account for the call to alloc.
	sub r4, sp, 8

	call memory.alloc
	ret
@endf _malloc


@func _free:
	; r0: 0 if it was able to deallocate, -1 otherwise.
	; r1: heap block to free.

	; Load the heap start address and call memory.free
	ldr r2, [heap_lower_limit]
	call memory.free
	ret
@endf _free

; ====== Text mode functions and data.

.section text

; ==== PutS: Prints a string on the screen.
@func _puts:
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
@endf _puts


; ==== IncXY: Given an (x,y) position, moves to the next position respecting
; screen boundaries.
@func incxy:
    ; r1: x-pos
    ; r2: y-pos
    add r1, r1, 1
    sub r21, r1, 100
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

.section data
    .equ kLineLength 2560 ; 640 * 4

.section text


; ==== PutC: Prints a character on the screen.
@func _putc:
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
@endf _putc

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

    ; Copy to vram.
	ldr r24, [memcpy32]
    call r24

    ; Flush using text mode.
    mov r26, 2
    call flush_video

    ; We are done.
    ret
@endf console_flush

@func console_set_cursor:
    ; if x < 0, set it to 0.
    jge r1, x_99
    mov r1, 0

x_99:
    ; if x >= 100, set it to 99
    sub r3, r1, 100
    jlt r3, y
    mov r3, 99

y:
    ; if y < 0, set it to 0
    jge r2, y_27
    mov r2, 0

y_27:
    ; if y >= 28, set it to 27
    sub r3, r2, 28
    jlt r3, done
    mov r2, 27

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
    mov r5, frame_buffer
	ldr r0, [text_putc]
    call r0
    ret
@endf console_putc

@func console_puts:
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_fcolor]
    ldri r4, [r0, console_bcolor]
	mov r5, frame_buffer
    ldr r7, [text_putc]
	ldr r0, [puts]
    call r0
    ret
@endf console_puts

@func console_print_cursor:
    mov r6, 0x2588
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_fcolor]
    ldri r4, [r0, console_bcolor]
    mov r5, frame_buffer
    ldr r0, [text_putc]
	call r0
    ret
@endf console_print_cursor

@func console_erase_cursor:
    mov r6, 0
    ldri r1, [r0, console_cursor_x]
    ldri r2, [r0, console_cursor_y]
    ldri r3, [r0, console_bcolor]
    ldri r4, [r0, console_bcolor]
    mov r5, frame_buffer
	ldr r0, [text_putc]
	call r0
    ret
@endf console_erase_cursor

@func console_next_cursor:
    ldri r1, [r0, console_cursor_x]
    add r1, r1, 1
    sub r2, r1, 100
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
    sub r3, r2, 28
    jlt r3, done_all

    strpi [sp, -4], r1
    call console_scroll_up
    ldrip r1, [sp, 4]
    mov r2, 27

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
    add r2, r1, 400

    ; End of framebuffer.
    ldri r3, [r0, sbuf_end]

    ; Get number of bytes.
    sub r3, r3, r2

    ; Convert to words.
    lsr r3, r3, 2

    ; Copy back skipping the first line.
	ldr r24, [memcpy32]
    call r24

    ; Erase last line.
    ldri r1, [r0, sbuf_end]
    sub r1, r1, 400
    mov r2, 100
    ldri r3, [r0, console_bcolor]
	ldr r24, [memset32]
    call r24

    ret
@endf console_scroll_up

@func console_prev_cursor:
    ldri r1, [r0, console_cursor_x]
    sub r1, r1, 1
    jge r1, done
    mov r1, 99

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
gvm:            .str "GVM Virtual Machine Version 0.1987"
ready:			.str "READY."
mem_av:			.str "Memory available:"
kibytes:        .str "Kilobytes"
recurring_reg:  .int 0x1200410
frame_buffer:   .array 11200
used_bytes:     .array 24

.section text

@func KERNEL_MAIN:
    ; Allocate space for console
	mov r1, console_size
	call _malloc

	jne r0, allocated
	; Not enough memory. Halt the cpu.
	halt

allocated:
    str [console_addr], r0

    ; Initialize console.
    mov r1,  frame_buffer
    mov r2, 400    ; 100 x 4 (size of text line in bytes.)
    mul r2, r2, 28 ; 28 (number of lines)
    add r2, r2, r1
    mov r3, 11
    mov r4, 0
    call console_init

    ; Print machine name
    ldr r0, [console_addr]
    mov r1, 32
    mov r2, 0
    call console_set_cursor

    mov r6, gvm
    call console_puts

	; Calculate memory available.
	ldr r1, [heap_curr_limit]
	sub r1, sp, r1
	; We want the amount in kilo bytes, so divide by 1024.
	lsr r1, r1, 10
	mov r2, used_bytes
	ldr r0, [itoa]
	call r0

	; Print memory available string.
	ldr r0, [console_addr]
    mov r1, 0
    mov r2, 1
    call console_set_cursor
	ldr r0, [console_addr]
    mov r6, mem_av
    call console_puts

    ; Change text color to white.
    ldr r0, [console_addr]
    mov r1, 15
    mov r2, 0
    call console_set_color

	; Print actual bytes available
	ldr r0, [console_addr]
	mov r1, 18
	mov r2, 1
	call console_set_cursor
	ldr r0, [console_addr]
	mov r6, used_bytes
	call console_puts

	; Print KiB suffix
	mov r1, used_bytes
	ldr r0, [strlen]
	call r0
	add r0, r0, 19

	mov r1, r0
	ldr r0, [console_addr]
	mov r2, 1
	call console_set_cursor
	ldr r0, [console_addr]
	mov r6, kibytes
	call console_puts



    ; Print ready sign.
	ldr r0, [console_addr]
    mov r1, 0
    mov r2, 3
    call console_set_cursor
	ldr r0, [console_addr]
    mov r6, ready
    call console_puts

    ; Print cursor
    ldr r0, [console_addr]
    mov r1, 0
    mov r2, 4
    call console_set_cursor
	ldr r0, [console_addr]
    call console_print_cursor

    ; Install our display updater.
    mov r0, USER_INTERFACE
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
	ldr r0, [getc]
	call r0
    jne r0, process_input
    ret

process_input:
    ; check if we have a control char. If we do, update ui accordingly and get next
    ; input.
	mov r1, r0
    call USER_control_chars
    jeq r0, done
    mov r6, r1

    ; Normal char, print.
    ldr r0, [console_addr]
    call console_putc

    ; Advance cursor.
    ldr r0, [console_addr]
    call console_next_cursor
	ldr r0, [console_addr]
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
	ldr r0, [console_addr]
    call console_prev_cursor
	ldr r0, [console_addr]
    call console_print_cursor
    mov r0, 0
    ret

enter:
    ; Erase cursor at current position
    ldr r0, [console_addr]
    call console_erase_cursor
	ldr r0, [console_addr]
    call console_nextline_cursor
	ldr r0, [console_addr]
    call console_print_cursor
    mov r0, 0
    ret
@endf USER_control_chars

; ============ HEAP START. DO NOT CHANGE THIS
.section data
heap_start: .int 0x0

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


