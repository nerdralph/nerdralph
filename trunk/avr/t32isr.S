
/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

.section .bss
; must define dummy values for variables or else no
; space will be reserved and all will refer to RAMSTART
ovfl_count:
            .byte 0

; __system_time defined in avr/time.h
.global __system_time
__system_time:
            .long 0

.section .text
.global TIMER2_COMPA_vect
TIMER2_COMPA_vect:
    push r16
    in r16, SREG
    push r16                        ; save SREG
    push ZH
    push ZL
    ldi ZL, lo8(ovfl_count)
    ldi ZH, hi8(ovfl_count)
    ld r16, Z
    subi r16, 250
    st Z+, r16
    ; increments seconds every 250th time when carry clear
loop:
    ld r16, Z
    sbci r16, -1                    ; subtract -1 = add 1
    st Z+, r16
    brcc loop
    pop ZL
    pop ZH
    pop r16
    out SREG, r16
    pop r16
    reti

