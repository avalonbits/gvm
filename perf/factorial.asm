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
@func fact:
  ; r0: where we return the result.
  ; r1: the number we want to call factorial on.
  mov r0, r1
  jle r0, base_done
  sub r0, r0, 1
  jeq r0, base_done

  strpi [sp, -4], r1
  sub r1, r1, 1
  call fact
  ldrip r3, [sp, 4]
  mul r0, r0, r3
  ret

base_done:
  mov r0, 1
  ret
@endf fact

.section data
func_calls: .int 0x1

.section text
@func benchmark:
  ldr r10, [func_calls]

loop:
  mov r1, 12
  call fact
  sub r10, r10, 1
  jne r10, loop
  halt
@endf benchmark
