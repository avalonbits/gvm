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


; ===== The acutal benchmark function.
@infunc benchmark:
  ldr r0, [loop_size]
  
loop:
  ldr r1, [mem]
  add r1, r1, 1
  lsl r1, r1, 4
  call update
  str [mem], r0
  ldr r2, [mem]
  mul r2, r1, r2
  jne r0, loop
  halt
@endf benchmark

@func update:
  stppi [sp, -8], r0, r1
  add r1, r1, 7
  str [mem], r1
  ldpip r0, r1, [sp, 8]
  sub r0, r0, 1
  ret
@endf update

.section data
mem: .int 0x0
loop_size: .int 0x1000000;FFFFFFF
