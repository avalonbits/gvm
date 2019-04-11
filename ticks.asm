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

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
  mov r2, 50000
loop:
  ldr r0, [timer_reg]
  ldr r1, [r0]
  sub r2, r2, 1
  jne r2, loop
@endf benchmark
halt
