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
timer_reg: .int 0x1200008
b_loop: .int 100000

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
  ldr r2, [b_loop]
  ldr r0, [timer_reg]
loop:
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  nop
  ldr r1, [r0]
  sub r2, r2, 20
  jne r2, loop
@endf benchmark
halt
