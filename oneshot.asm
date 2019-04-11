; GVM Benchmark
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset and timer.
interrupt_table:
  jmp benchmark  ; Reset interrupt.
  jmp oneshot    ; Timer interrupt.
  ret            ; Input intterupt.


.section data
timer_reg: .int 0x1200008
oneshot_reg: .int 0x120000C

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
  ldr r0, [oneshot_reg]
  mov r1, 10000
  str [r0], r1
  wfi
  halt
@endf benchmark

@func oneshot:
  ldr r1, [oneshot_reg]
  ldr r1, [r1]
  ret
@endf oneshot
