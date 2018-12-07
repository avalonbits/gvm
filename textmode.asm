; Text mode rom.
.org 0xE2410  ; Load from this address.

.section data
  ; Textmode supports 16 colors. Colors are encoded in RGBA with R starting
  ; in LSB.

color16:	    ;0xAABBGGRR
  .int C_BLACK   0xFF000000
  .int C_MAROON  0xFF000080
  .int C_GREEN   0xFF008000
  .int C_OLIVE   0xFF008080
  .int C_NAVY    0xFF800000
  .int C_PURPLE  0xFF800080
  .int C_TEAL    0xFF808000
  .int C_SILVER  0xFFC0C0C0
  .int C_GREY    0xFF808080
  .int C_RED     0xFF0000FF
  .int C_LIME    0xFF00FF00
  .int C_YELLOW  0xFF00FFFF
  .int C_BLUE    0xFFFF0000
  .int C_FUCHSIA 0XFFFF00FF
  .int C_AQUA    0xFFFFFF00
  .int C_WHITE   0xFFFFFFFF

  .int LINE_LENGTH 2880  ; 720 * 4 ->  720 pixels, 32bpp


.section text
; writec will write a character to the framebuffer.
; r1: The character to write (latin1 unicode code).
; r2: Position in the framebuffer to write character to.
; r3: Color of the character (0-15).
writec:
  ; First we need to get the correct color value
  mov r0, color16   ; Go to start of color table.
  lsl r3, r3, 2     ; r3 * 4 because every color is 4 bytes.
  add r3, r3, r0    ; r0 + r3 will give us the addres ofcorrect color.
  ldr r3, [r3]      ; Load color value from r3 address into r3.

  lsl r1, r1, 4     ; r1 * 16 because each char uses 4 words.
  sub r1, r1, 4     ; sub one word because we will start at word 0 in loop.

  ; Start address of char rom is 0xE1400
  mov r0, 0xE1400   ; Start address of character rom.
  mov r10, 5        ; Counts number of char words left to process.

wordloop: 
  ; while (r10 > 0)
  sub r10, r10, 1
  jeq r10, wrice_done  ; r10 == 0 -> We are done.

  ; We advance 5 times se we need to subtract 1 to stay correct.
  sub r1, r1, LINE_LENGTH
  add r1, r1, 4     ; next char word.
  add r4, r0, r1    ; r0 + r1 will give us the correct word addr for the char.
  ldr r4, [r4]      ; Load the char word from r4 address into r4.

  mov r5, -1        ; bits to shift right. We will shift the 32 bits.

inword_loop:
  ; Next line in framebuffer.
  add r2, r2, LINE_LENGTH

  mov r7, 32        ; Start at x pos + 8. 32 because each position is 4 bytes wide.

next_pos:
  sub r7, r7, 4     ; next position in framebuffer
  ; while (r13 > 0)
  add r13, r7, 4
  jeq r13, inword_loop ; r13 == 0 -> We are done with line.

  add r5, r5, 1     ; Next bit to process.
  ; while (r5 < 32); 
  sub r6, r5, 32    
  jeq r6, wordloop  ; r6 == 32 -> done with word, get next.

  lsr r6, r4, r5    ; r4 >> r5 bits
  and r6, r6, 1     ; Check if bit is active.
  jeq r6, next_pos  ; Bit is not active, try next position.

  ; Bit is active, write the pixel.
  add r6, r2, r7    ; r2 + r7 gives the framebuffer position.
  str [r6], r3      ; Set pixel at framebuffer position to color r3.
  jmp next_pos

writec_done:
  ret

