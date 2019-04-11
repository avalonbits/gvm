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
  str [r0], 10000
  wfi
@endf benchmark
halt

@func oneshot:
@endf oneshot
