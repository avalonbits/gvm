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

.org 0x0
.section text

; Jump table for interrupt handlers. Each address is for a specific interrupt.
interrupt_table:
    jmp reset_handler     ; Reset interrupt.
	ret                   ; Timer1 interrupt.
	ret                   ; Input handler.
	ret                   ; Video refresh handler (recurring timer1).
	ret                   ; Timer2 interrupt.
	ret                   ; Recurring timer2 interrupt.
	ret                   ; Video interrupt.

.org 0x400
.section data
memset:     .int memory.set
memset2:    .int memory.set2
memset4:    .int memory.set4
memset32:   .int memory.set32
memcpy:     .int memory.copy
memcpy2:    .int memory.copy2
memcpy4:    .int memory.copy4
memcpy32:   .int memory.copy32
; ===== Kernel function table.

.org 0x2400

.include "includes/v2/memory.asm" as memory
.include "includes/v2/textmode.asm" as textmode

.section text
; ==== Reset interrupt handler.
reset_handler:
	call textmode.init
	mov r0, 0x42
	call textmode.putc
    ; Now jump to main kernel code.
    jmp KERNEL_MAIN

KERNEL_MAIN:
	wfi
	halt

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


