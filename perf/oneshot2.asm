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
    jmp oneshot2   ; Timer2 interrupt.


.section data
oneshot2_reg: .int 0x1200414

.section text
; ===== The acutal benchmark function.
@infunc benchmark:
    ldr r0, [oneshot2_reg]
    mov r1, 9000
    str [r0], r1
    wfi
    halt
@endf benchmark

@func oneshot2:
    ldr r1, [oneshot2_reg]
    ldr r1, [r1]
    ret
@endf oneshot2
