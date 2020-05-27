; hardware accelerated UART using Timer0 output compare

; PU_T0_SUB = Timer0 prescaled count per bit negated for subi

#define __SFR_OFFSET 0
#include <avr/io.h>

#define CLR_ON_MATCH (1<<COM0A1)
#define SET_ON_MATCH ((1<<COM0A1) | (1<<COM0A0))

; t13 and t85 use TIMSK0/TIMSK but same address
.equiv T0INTMSK, 0x39

; tx bit ISR 29c incl reti
TIM0_COMPA_vect:                ; t13
TIMER0_COMPA_vect:              ; t85
    push r24
    in r24, SREG
    push r24
    lds r24, wgm_txdata
    cpi r24, 0
    brne morebits
    in r24, T0INTMSK
    cbr r24, 1<<OCIE0A
    out r24, T0INTMSK               ; disable ISR
    rjmp donetx
morebits:
    bst r24, 0                      ; save LSB in T
    lsr r24
    sts wgm_txdata, r24
    ;ldi r24, CLR_ON_MATCH
    in r24, TCCR0A
    bld r24, COM0A0                 ; set on match when T set
    out TCCR0A, r24
donetx:
    in r24, OCR0A
    subi r24, TICKS_PER_BIT_NEG
    out OCR0A, r24                  ; set time for next bit
epilogue:
    pop r24
    out SREG, r24
    pop r24
reti

; rx bit ISR 26c incl reti
; bit sampled 9 cycles into ISR
TIM0_COMPB_vect:                    ; t13
TIMER0_COMPB_vect:                  ; t85
    push r24
    in r24, SREG
    push r24
    lds r24, wgm_rxdata
    bst r24, 0                      ; save LSB in T
    sbic PINB, WGMRXBIT
    ori r24, 0x80                   ; rx bit is a 1
    sts wgm_rxdata, r24
    in r24, OCR0B
    subi r24, TICKS_PER_BIT_NEG
    out OCR0B, r24                  ; set time for next bit
    brtc epilogue                   ; still more bits to rx
    in r24, T0INTMSK
    cbr r24, 1<<OCIE0B
    out r24, T0INTMSK               ; disable ISR
    rjmp epilogue

.section .noinit, "aw", @nobits
.global pu_outchar
pu_outchar: .zero 1
