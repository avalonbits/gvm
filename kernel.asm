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
vram_reg:   .int 0x1200000
vram_start: .int 0x101F000

.section text

; ==== Reset interrupt handler.
reset_handler:
    ; Clear input register
    ldr r1, [input_value_addr]
    str [r1], rZ

    ; Clear user input vector address.
    str [input_jump_addr], rZ

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

.section data

; Text mode supports 16 colors and all functions require a foreground and a
; background color.
text_colors:
    .int 0xFF000000 ;   0 - Black
    .int 0xFF000008 ;   1 - Maroon
    .int 0xFF000800 ;   2 - Green
    .int 0xFF000808 ;   3 - Olive
    .int 0xFF080000 ;   4 - Navy
    .int 0xFF080008 ;   5 - Purple
    .int 0xFF080800 ;   6 - Teal
    .int 0xFF0C0C0C ;   7 - Silver
    .int 0xFF080808 ;   8 - Grey
    .int 0xFF0000FF ;   9 - Red
    .int 0xFF00FF00 ;  10 - Lime
    .int 0xFF00FFFF ;  11 - Yellow
    .int 0xFFFF0000 ;  12 - Blue
    .int 0xFFFF00FF ;  13 - Fuchsia
    .int 0xFFFFFF00 ;  14 - Aqua
    .int 0xFFFFFFFF ;  15 - White
    .int 0xFF000000 ;  16 - Grey0
    .int 0xFFF50000 ;  17 - NavyBlue
    .int 0xFF780000 ;  18 - DarkBlue
    .int 0xFFFA0000 ;  19 - Blue3
    .int 0xFF7D0000 ;  20 - Blue3
    .int 0xFFFF0000 ;  21 - Blue1
    .int 0xFF00F500 ;  22 - DarkGreen
    .int 0xFFF5F500 ;  23 - DeepSkyBlue4
    .int 0xFF78F500 ;  24 - DeepSkyBlue4
    .int 0xFFFAF500 ;  25 - DeepSkyBlue4
    .int 0xFF7DF500 ;  26 - DodgerBlue3
    .int 0xFFFFF500 ;  27 - DodgerBlue2
    .int 0xFF007800 ;  28 - Green4
    .int 0xFFF57800 ;  29 - SpringGreen4
    .int 0xFF787800 ;  30 - Turquoise4
    .int 0xFFFA7800 ;  31 - DeepSkyBlue3
    .int 0xFF7D7800 ;  32 - DeepSkyBlue3
    .int 0xFFFF7800 ;  33 - DodgerBlue1
    .int 0xFF00FA00 ;  34 - Green3
    .int 0xFFF5FA00 ;  35 - SpringGreen3
    .int 0xFF78FA00 ;  36 - DarkCyan
    .int 0xFFFAFA00 ;  37 - LightSeaGreen
    .int 0xFF7DFA00 ;  38 - DeepSkyBlue2
    .int 0xFFFFFA00 ;  39 - DeepSkyBlue1
    .int 0xFF007D00 ;  40 - Green3
    .int 0xFFF57D00 ;  41 - SpringGreen3
    .int 0xFF787D00 ;  42 - SpringGreen2
    .int 0xFFFA7D00 ;  43 - Cyan3
    .int 0xFF7D7D00 ;  44 - DarkTurquoise
    .int 0xFFFF7D00 ;  45 - Turquoise2
    .int 0xFF00FF00 ;  46 - Green1
    .int 0xFFF5FF00 ;  47 - SpringGreen2
    .int 0xFF78FF00 ;  48 - SpringGreen1
    .int 0xFFFAFF00 ;  49 - MediumSpringGreen
    .int 0xFF7DFF00 ;  50 - Cyan2
    .int 0xFFFFFF00 ;  51 - Cyan1
    .int 0xFF0000F5 ;  52 - DarkRed
    .int 0xFFF500F5 ;  53 - DeepPink4
    .int 0xFF7800F5 ;  54 - Purple4
    .int 0xFFFA00F5 ;  55 - Purple4
    .int 0xFF7D00F5 ;  56 - Purple3
    .int 0xFFFF00F5 ;  57 - BlueViolet
    .int 0xFF00F5F5 ;  58 - Orange4
    .int 0xFFF5F5F5 ;  59 - Grey37
    .int 0xFF78F5F5 ;  60 - MediumPurple4
    .int 0xFFFAF5F5 ;  61 - SlateBlue3
    .int 0xFF7DF5F5 ;  62 - SlateBlue3
    .int 0xFFFFF5F5 ;  63 - RoyalBlue1
    .int 0xFF0078F5 ;  64 - Chartreuse4
    .int 0xFFF578F5 ;  65 - DarkSeaGreen4
    .int 0xFF7878F5 ;  66 - PaleTurquoise4
    .int 0xFFFA78F5 ;  67 - SteelBlue
    .int 0xFF7D78F5 ;  68 - SteelBlue3
    .int 0xFFFF78F5 ;  69 - CornflowerBlue
    .int 0xFF00FAF5 ;  70 - Chartreuse3
    .int 0xFFF5FAF5 ;  71 - DarkSeaGreen4
    .int 0xFF78FAF5 ;  72 - CadetBlue
    .int 0xFFFAFAF5 ;  73 - CadetBlue
    .int 0xFF7DFAF5 ;  74 - SkyBlue3
    .int 0xFFFFFAF5 ;  75 - SteelBlue1
    .int 0xFF007DF5 ;  76 - Chartreuse3
    .int 0xFFF57DF5 ;  77 - PaleGreen3
    .int 0xFF787DF5 ;  78 - SeaGreen3
    .int 0xFFFA7DF5 ;  79 - Aquamarine3
    .int 0xFF7D7DF5 ;  80 - MediumTurquoise
    .int 0xFFFF7DF5 ;  81 - SteelBlue1
    .int 0xFF00FFF5 ;  82 - Chartreuse2
    .int 0xFFF5FFF5 ;  83 - SeaGreen2
    .int 0xFF78FFF5 ;  84 - SeaGreen1
    .int 0xFFFAFFF5 ;  85 - SeaGreen1
    .int 0xFF7DFFF5 ;  86 - Aquamarine1
    .int 0xFFFFFFF5 ;  87 - DarkSlateGray2
    .int 0xFF000078 ;  88 - DarkRed
    .int 0xFFF50078 ;  89 - DeepPink4
    .int 0xFF780078 ;  90 - DarkMagenta
    .int 0xFFFA0078 ;  91 - DarkMagenta
    .int 0xFF7D0078 ;  92 - DarkViolet
    .int 0xFFFF0078 ;  93 - Purple
    .int 0xFF00F578 ;  94 - Orange4
    .int 0xFFF5F578 ;  95 - LightPink4
    .int 0xFF78F578 ;  96 - Plum4
    .int 0xFFFAF578 ;  97 - MediumPurple3
    .int 0xFF7DF578 ;  98 - MediumPurple3
    .int 0xFFFFF578 ;  99 - SlateBlue1
    .int 0xFF007878 ; 100 - Yellow4
    .int 0xFFF57878 ; 101 - Wheat4
    .int 0xFF787878 ; 102 - Grey53
    .int 0xFFFA7878 ; 103 - LightSlateGrey
    .int 0xFF7D7878 ; 104 - MediumPurple
    .int 0xFFFF7878 ; 105 - LightSlateBlue
    .int 0xFF00FA78 ; 106 - Yellow4
    .int 0xFFF5FA78 ; 107 - DarkOliveGreen3
    .int 0xFF78FA78 ; 108 - DarkSeaGreen
    .int 0xFFFAFA78 ; 109 - LightSkyBlue3
    .int 0xFF7DFA78 ; 110 - LightSkyBlue3
    .int 0xFFFFFA78 ; 111 - SkyBlue2
    .int 0xFF007D78 ; 112 - Chartreuse2
    .int 0xFFF57D78 ; 113 - DarkOliveGreen3
    .int 0xFF787D78 ; 114 - PaleGreen3
    .int 0xFFFA7D78 ; 115 - DarkSeaGreen3
    .int 0xFF7D7D78 ; 116 - DarkSlateGray3
    .int 0xFFFF7D78 ; 117 - SkyBlue1
    .int 0xFF00FF78 ; 118 - Chartreuse1
    .int 0xFFF5FF78 ; 119 - LightGreen
    .int 0xFF78FF78 ; 120 - LightGreen
    .int 0xFFFAFF78 ; 121 - PaleGreen1
    .int 0xFF7DFF78 ; 122 - Aquamarine1
    .int 0xFFFFFF78 ; 123 - DarkSlateGray1
    .int 0xFF0000FA ; 124 - Red3
    .int 0xFFF500FA ; 125 - DeepPink4
    .int 0xFF7800FA ; 126 - MediumVioletRed
    .int 0xFFFA00FA ; 127 - Magenta3
    .int 0xFF7D00FA ; 128 - DarkViolet
    .int 0xFFFF00FA ; 129 - Purple
    .int 0xFF00F5FA ; 130 - DarkOrange3
    .int 0xFFF5F5FA ; 131 - IndianRed
    .int 0xFF78F5FA ; 132 - HotPink3
    .int 0xFFFAF5FA ; 133 - MediumOrchid3
    .int 0xFF7DF5FA ; 134 - MediumOrchid
    .int 0xFFFFF5FA ; 135 - MediumPurple2
    .int 0xFF0078FA ; 136 - DarkGoldenrod
    .int 0xFFF578FA ; 137 - LightSalmon3
    .int 0xFF7878FA ; 138 - RosyBrown
    .int 0xFFFA78FA ; 139 - Grey63
    .int 0xFF7D78FA ; 140 - MediumPurple2
    .int 0xFFFF78FA ; 141 - MediumPurple1
    .int 0xFF00FAFA ; 142 - Gold3
    .int 0xFFF5FAFA ; 143 - DarkKhaki
    .int 0xFF78FAFA ; 144 - NavajoWhite3
    .int 0xFFFAFAFA ; 145 - Grey69
    .int 0xFF7DFAFA ; 146 - LightSteelBlue3
    .int 0xFFFFFAFA ; 147 - LightSteelBlue
    .int 0xFF007DFA ; 148 - Yellow3
    .int 0xFFF57DFA ; 149 - DarkOliveGreen3
    .int 0xFF787DFA ; 150 - DarkSeaGreen3
    .int 0xFFFA7DFA ; 151 - DarkSeaGreen2
    .int 0xFF7D7DFA ; 152 - LightCyan3
    .int 0xFFFF7DFA ; 153 - LightSkyBlue1
    .int 0xFF00FFFA ; 154 - GreenYellow
    .int 0xFFF5FFFA ; 155 - DarkOliveGreen2
    .int 0xFF78FFFA ; 156 - PaleGreen1
    .int 0xFFFAFFFA ; 157 - DarkSeaGreen2
    .int 0xFF7DFFFA ; 158 - DarkSeaGreen1
    .int 0xFFFFFFFA ; 159 - PaleTurquoise1
    .int 0xFF00007D ; 160 - Red3
    .int 0xFFF5007D ; 161 - DeepPink3
    .int 0xFF78007D ; 162 - DeepPink3
    .int 0xFFFA007D ; 163 - Magenta3
    .int 0xFF7D007D ; 164 - Magenta3
    .int 0xFFFF007D ; 165 - Magenta2
    .int 0xFF00F57D ; 166 - DarkOrange3
    .int 0xFFF5F57D ; 167 - IndianRed
    .int 0xFF78F57D ; 168 - HotPink3
    .int 0xFFFAF57D ; 169 - HotPink2
    .int 0xFF7DF57D ; 170 - Orchid
    .int 0xFFFFF57D ; 171 - MediumOrchid1
    .int 0xFF00787D ; 172 - Orange3
    .int 0xFFF5787D ; 173 - LightSalmon3
    .int 0xFF78787D ; 174 - LightPink3
    .int 0xFFFA787D ; 175 - Pink3
    .int 0xFF7D787D ; 176 - Plum3
    .int 0xFFFF787D ; 177 - Violet
    .int 0xFF00FA7D ; 178 - Gold3
    .int 0xFFF5FA7D ; 179 - LightGoldenrod3
    .int 0xFF78FA7D ; 180 - Tan
    .int 0xFFFAFA7D ; 181 - MistyRose3
    .int 0xFF7DFA7D ; 182 - Thistle3
    .int 0xFFFFFA7D ; 183 - Plum2
    .int 0xFF007D7D ; 184 - Yellow3
    .int 0xFFF57D7D ; 185 - Khaki3
    .int 0xFF787D7D ; 186 - LightGoldenrod2
    .int 0xFFFA7D7D ; 187 - LightYellow3
    .int 0xFF7D7D7D ; 188 - Grey84
    .int 0xFFFF7D7D ; 189 - LightSteelBlue1
    .int 0xFF00FF7D ; 190 - Yellow2
    .int 0xFFF5FF7D ; 191 - DarkOliveGreen1
    .int 0xFF78FF7D ; 192 - DarkOliveGreen1
    .int 0xFFFAFF7D ; 193 - DarkSeaGreen1
    .int 0xFF7DFF7D ; 194 - Honeydew2
    .int 0xFFFFFF7D ; 195 - LightCyan1
    .int 0xFF0000FF ; 196 - Red1
    .int 0xFFF500FF ; 197 - DeepPink2
    .int 0xFF7800FF ; 198 - DeepPink1
    .int 0xFFFA00FF ; 199 - DeepPink1
    .int 0xFF7D00FF ; 200 - Magenta2
    .int 0xFFFF00FF ; 201 - Magenta1
    .int 0xFF00F5FF ; 202 - OrangeRed1
    .int 0xFFF5F5FF ; 203 - IndianRed1
    .int 0xFF78F5FF ; 204 - IndianRed1
    .int 0xFFFAF5FF ; 205 - HotPink
    .int 0xFF7DF5FF ; 206 - HotPink
    .int 0xFFFFF5FF ; 207 - MediumOrchid1
    .int 0xFF0078FF ; 208 - DarkOrange
    .int 0xFFF578FF ; 209 - Salmon1
    .int 0xFF7878FF ; 210 - LightCoral
    .int 0xFFFA78FF ; 211 - PaleVioletRed1
    .int 0xFF7D78FF ; 212 - Orchid2
    .int 0xFFFF78FF ; 213 - Orchid1
    .int 0xFF00FAFF ; 214 - Orange1
    .int 0xFFF5FAFF ; 215 - SandyBrown
    .int 0xFF78FAFF ; 216 - LightSalmon1
    .int 0xFFFAFAFF ; 217 - LightPink1
    .int 0xFF7DFAFF ; 218 - Pink1
    .int 0xFFFFFAFF ; 219 - Plum1
    .int 0xFF007DFF ; 220 - Gold1
    .int 0xFFF57DFF ; 221 - LightGoldenrod2
    .int 0xFF787DFF ; 222 - LightGoldenrod2
    .int 0xFFFA7DFF ; 223 - NavajoWhite1
    .int 0xFF7D7DFF ; 224 - MistyRose1
    .int 0xFFFF7DFF ; 225 - Thistle1
    .int 0xFF00FFFF ; 226 - Yellow1
    .int 0xFFF5FFFF ; 227 - LightGoldenrod1
    .int 0xFF78FFFF ; 228 - Khaki1
    .int 0xFFFAFFFF ; 229 - Wheat1
    .int 0xFF7DFFFF ; 230 - Cornsilk1
    .int 0xFFFFFFFF ; 231 - Grey100
    .int 0xFF808080 ; 232 - Grey3
    .int 0xFF212121 ; 233 - Grey7
    .int 0xFFC1C1C1 ; 234 - Grey11
    .int 0xFF626262 ; 235 - Grey15
    .int 0xFF030303 ; 236 - Grey19
    .int 0xFFA3A3A3 ; 237 - Grey23
    .int 0xFF444444 ; 238 - Grey27
    .int 0xFFE4E4E4 ; 239 - Grey30
    .int 0xFF858585 ; 240 - Grey35
    .int 0xFF262626 ; 241 - Grey39
    .int 0xFFC6C6C6 ; 242 - Grey42
    .int 0xFF676767 ; 243 - Grey46
    .int 0xFF080808 ; 244 - Grey50
    .int 0xFFA8A8A8 ; 245 - Grey54
    .int 0xFF494949 ; 246 - Grey58
    .int 0xFFE9E9E9 ; 247 - Grey62
    .int 0xFF8A8A8A ; 248 - Grey66
    .int 0xFF2B2B2B ; 249 - Grey70
    .int 0xFFCBCBCB ; 250 - Grey74
    .int 0xFF6C6C6C ; 251 - Grey78
    .int 0xFF0D0D0D ; 252 - Grey82
    .int 0xFFADADAD ; 253 - Grey85
    .int 0xFF4E4E4E ; 254 - Grey89
    .int 0xFFEEEEEE ; 255 - Grey93

text_colors_addr: .int text_colors

.section text

; ==== TextPutS: Prints a string on the screen in text mode.
@func text_puts:
    ; r1: Address to string start.
    ; r2: x-pos start.
    ; r3: y-pos start.
    ; r4: foreground color.
    ; r5: background color.
    ; r6: frame buffer address.

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
    strpi [sp, -4], r6
    call text_putc

    ; Restore context.
    ldrip r6, [sp, 4]
    ldpip r4, r5, [sp, 8]
    ldpip r2, r3, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Next char in same word
    lsr r1, r22, 16
    jeq r1, done

    stppi [sp, -8], r2, r3
    stppi [sp, -8], r4, r5
    strpi [sp, -4], r6

    call text_putc

    ldrip r6, [sp, 4]
    ldpip r4, r5, [sp, 8]
    ldpip r2, r3, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Advance to next string word.
    add r20, r20, 4
    jmp loop

done:
    ret
@endf text_puts


; ==== PutS: Prints a string on the screen.
@func puts:
    ; r1: Address to string start.
    ; r2: x-pos start.
    ; r3: y-pos start.
    ; r4: foreground color.
    ; r5: background color.
    ; r6: frame buffer address.

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
    strpi [sp, -4], r6
    call putc

    ; Restore context.
    ldrip r6, [sp, 4]
    ldpip r4, r5, [sp, 8]
    ldpip r2, r3, [sp, 8]

    ; Update (x,y)
    call incxy

    ; Next char in same word
    lsr r1, r22, 16
    jeq r1, done

    stppi [sp, -8], r2, r3
    stppi [sp, -8], r4, r5
    strpi [sp, -4], r6

    call putc

    ldrip r6, [sp, 4]
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
    sub r21, r2, 96
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

; ==== TextPutC: Prints a charcater on the screen in text mode.
@func text_putc:
    ; r1: Character unicode value
    ; r2: x-pos
    ; r3: y-pos
    ; r4: foreground color
    ; r5: background color
    ; r6: framebuffer start.

	; We calculate position in framebuffer using the formula
	; pos(x,y) x*4 + fb_addr + y * 96 * 4
	lsl r2, r2, 2
	lsl r3, r3, 7
	mul r3, r3, 3
	add r6, r6, r2
	add r6, r6, r3

	; In text mode, we write a word with 2 bytes for char, 1 byte for fcolor
	; and 1 byte for bcolor. char is in r1, so we just need to write the colors.
	and r1, r1, 0xFFFF
	and r4, r4, 0xFF
	lsl r4, r4, 16
	orr r1, r1, r4

	and r5, r5, 0xFF;
	lsl r5, r5, 24
	orr r1, r1, r5

	str [r6], r1
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
    .equ sbuf_min_line 0
    .equ sbuf_sline 4
    .equ sbuf_eline 8
    .equ sbuf_max_line 12
    .equ sbuf_x_pos 16
    .equ sbuf_size 20

    ; struct console
    .equ console_sbuf 0
    .equ console_fcolor 20
    .equ console_bcolor 24
    .equ console_cursor_x 28
    .equ console_cursor_y 32
    .equ console_size 36

.section text

@func sbuf_init:
    ; r0: ptr to start of sbuf
    ; r1: ptr to start of frame buffer.
    ; r2: ptr to last line of frame buffer.
    stri [r0, sbuf_min_line], r1
    stri [r0, sbuf_sline], r1
    stri [r0, sbuf_eline], r2
    stri [r0, sbuf_max_line], r2
    stri [r0, sbuf_x_pos], rZ
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
    ; Algorithm to update display:
    ; 1) if sbuf_sline < sbuf_eline then we just copy everything between them.
    ldri r1, [r0, sbuf_sline]
    ldri r2, [r0, sbuf_eline]
    sub r3, r2, r1
    jge r3, copy_top_bottom

copy_top_bottom:
    lsr r3, r3, 2
    ldr r1, [vram_start]
    ldri r2, [r0, sbuf_sline]

	; Before we copy, we need to wait for video to be ready.
	call wait_video
    call memcpy32

	; Flush using text mode.
	mov r26, 2
    call flush_video

    ret
@endf console_flush

@func console_set_cursor:
    ; if x < 0, set it to 0.
    jge r1, x_95
    mov r1, rZ

x_95:
    ; if x >= 96, set it to 95
    sub r3, r1, 96
    jlt r3, y
    mov r3, 95

y:
    ; if y < 0, set it to 0
    jge r2, y_26
    mov r2, rZ

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
    mov r2, rZ

done:
    stri [r0, console_fcolor], r1
    stri [r0, console_bcolor], r2
    ret
@endf console_set_color

@func console_putc:
    ldri r2, [r0, console_cursor_x]
    ldri r3, [r0, console_cursor_y]
    ldri r4, [r0, console_fcolor]
    ldri r5, [r0, console_bcolor]
    ldr r6, [fb_addr]

	call text_putc
    ret
@endf console_putc

@func console_puts:
    ldri r2, [r0, console_cursor_x]
    ldri r3, [r0, console_cursor_y]
    ldri r4, [r0, console_fcolor]
    ldri r5, [r0, console_bcolor]
    ldr r6, [fb_addr]
    call text_puts
    ret
@endf console_puts

@func console_print_cursor:
    mov r1, 0x2588
    ldri r2, [r0, console_cursor_x]
    ldri r3, [r0, console_cursor_y]
    ldri r4, [r0, console_fcolor]
    ldri r5, [r0, console_bcolor]
    ldr r6, [fb_addr]
    call text_putc
    ret
@endf console_print_cursor

@func console_erase_cursor:
	mov r1, rZ
    ldri r2, [r0, console_cursor_x]
    ldri r3, [r0, console_cursor_y]
    ldri r4, [r0, console_bcolor]
	ldri r5, [r0, console_bcolor]
	ldr r6, [fb_addr]
    call text_putc
    ret
@endf console_erase_cursor

@func console_next_cursor:
    ldri r1, [r0, console_cursor_x]
    add r1, r1, 1
    sub r2, r1, 96
    jne r2, done

    mov r1, rZ
    ldri r2, [r0, console_cursor_y]
    sub r3, r2, 27
    jne r3, done_all

    strpi [sp, -4], r1
    call console_scroll_up
    ldrip r1, [sp, 4]
    jmp done

done_all:
    add r2, r2, 1
    stri [r0, console_cursor_y], r2

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_next_cursor

@func console_nextline_cursor:
    mov r1, rZ
    ldri r2, [r0, console_cursor_y]
    sub r3, r2, 27
    jne r3, done_all

    strpi [sp, -4], r1
    call console_scroll_up
    ldrip r1, [sp, 4]
    jmp done

done_all:
    add r2, r2, 1
    stri [r0, console_cursor_y], r2

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_nextline_cursor

@func console_scroll_up:
    ldri r1, [r0, sbuf_sline]
    mov r2, 6144
    add r2, r2, r1
    ldri r3, [r0, sbuf_eline]
    sub r3, r3, r2
    lsr r3, r3, 2
    call memcpy32
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
    mov r1, rZ

done:
    stri [r0, console_cursor_x], r1
    ret
@endf console_prev_cursor


.section data
gvm: .str "GVM Virtual Machine Version 0.01"
gvm_addr: .int gvm
ready: .str "READY."
ready_addr: .int ready
recurring_reg: .int 0x1200010
UI_addr: .int USER_INTERFACE
frame_buffer: .array 10368
fb_addr: .int frame_buffer

.section text

@func MAIN:
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
    mov r1, 24
    mov r2, 0
    call console_set_cursor

    ldr r1, [gvm_addr]
    call console_puts

    ; Change text color to white.
    mov r0, sp
    mov r1, 36
    mov r2, 0
    call console_set_color

    ; Print ready sign.
    mov r1, 0
    mov r2, 2
    call console_set_cursor
    ldr r1, [ready_addr]
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


.org 0x1100000

.embed "./unicode16.chrom"
