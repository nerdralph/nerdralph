/* Ralph Doncaster 2014 public domain software
 * 1s timer ISR
 */

; needed for <avr/io.h> to give io constant addresses
#define __SFR_OFFSET 0
#include <avr/io.h>

; 1 byte variable in RAM
.lcomm ovfl_count, 1
.global ovfl_count

; 4 byte (long) global variable in RAM
; __system_time to match definition avr/time.h
.lcomm __system_time, 4
.global __system_time

.section .text
#define tmp8 r24 

.global TIMER2_COMPA_vect
TIMER2_COMPA_vect:
    push tmp8
    in tmp8, SREG
    push tmp8                        ; save SREG
    push ZH
    push ZL
#ifdef DEBUG
    sbi PORTD, 2                     ; toggle for debug
#endif
    ldi ZL, lo8(ovfl_count)
    ldi ZH, hi8(ovfl_count)
    ld tmp8, Z                       ; load seconds fraction
    cpi tmp8, 0                      ; skip counts 1-6
    brne adder
    ldi tmp8, 6                      ; reset fractions counter
adder:
    sbci tmp8, -1                    ; subtract -1 = add 1
    st Z+, tmp8
    ld tmp8, Z
    brcc adder
    pop ZL
    pop ZH
    pop tmp8
    out SREG, tmp8
    pop tmp8
    reti

