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
    ret            ; Recurring timer interrupt.
    ret            ; Timer2 interrupt.
    jmp recurring2 ; Recurring timer2 interrupt.


.section data
recurring2_reg: .int 0x1200018

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    mov r2, 0
    ldr r0, [recurring2_reg]
    mov r1, 30  ; 30Hz recurring2 timer.
    str [r0], r1

  ; Nothing more to do in the main thread. Everything will
  ; be handled in the recurring2 timer interrupt handler.
loop: wfi
      jmp loop
@endf benchmark

@func recurring2:
    add r2, r2, 1
    sub r3, r2, 270  ; 9 seconds.
    jeq r3, done
    ret
done:
    halt
@endf recurring2
