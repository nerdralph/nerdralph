/* Ralph Doncaster 2018 public domain software
 * timer ISR counter
 */

; needed for <avr/io.h> to give io constant addresses
#define __SFR_OFFSET 0
#include <avr/io.h>

.section .bss
.global __do_clear_bss

; 4 byte (long) global variable in RAM
.lcomm wdt_interrupt_counter, 4
.global wdt_interrupt_counter

.section .text

#define tmp1 r16
#define tmp2 r17

.global WDT_vect
WDT_vect:
    push ZL
    in ZL, SREG
    push ZL                             ; save SREG
    push tmp1
    ldi ZL, lo8(wdt_interrupt_counter)  ; must be 8-byte aligned
    ld tmp1, Z
    sbci tmp1, -19
    rjmp save
add:
    ld tmp1, Z
    sbci tmp1, -1
save:
    st Z+, tmp1
    brcc add
    pop tmp1
    pop ZL
    out SREG, ZL
    pop ZL
    reti
