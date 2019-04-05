; GVM Benchmark
; by Igor Cananea (icc@avalonbits.com

.org 0x0
.section text

; Jump table for interrupt handlers. For the benchmark, we want to ignore any
; interrupts except for reset.
interrupt_table:
  jmp benchmark  ; Reset interrupt.
  ret            ; Timer interrupt.
  ret            ; Input intterupt.


; ===== The acutal benchmark function.
@func benchmark:
  ldr r0, [loop_size]
  
loop:
  jeq r0, done
  ldr r1, [mem]
  add r1, r1, 1
  lsl r1, r1, 4
  div r1, r1, 2
  str [mem], r1
  sub r0, r0, 1
  str [mem], r0
  ldr r2, [mem]
  mul r2, r1, r2
  jmp loop

done:
@endf benchmark
halt

.section data
mem: .int 0x0
loop_size: .int 10000000
