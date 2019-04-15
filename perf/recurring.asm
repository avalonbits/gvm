; GVM Benchmark
; by Igor Cananea (icc@avalonbits.com)

.org 0x0
.section text

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset and timer.
interrupt_table:
  jmp benchmark  ; Reset interrupt.
  ret            ; Timer interrupt.
  ret            ; Input intterupt.
  jmp recurring  ; Recurring timer interrupt.


.section data
recurring_reg: .int 0x1200010

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
  mov r2, 0
  ldr r0, [recurring_reg]
  mov r1, 60  ; 60Hz recurring timer.
  str [r0], r1

  ; Nothing more to do in the main thread. Everything will
  ; be handled in the recurring timer interrupt handler.
loop: wfi
      jmp loop
@endf benchmark

@func recurring:
  add r2, r2, 1
  sub r3, r2, 600  ;  10 seconds.
  jeq r3, done
  ret
done:
  halt
@endf recurring
