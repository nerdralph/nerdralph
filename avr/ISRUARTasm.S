; Tx UART ISR 21 instr incl reti, 29c ttl + 4ISR + 2 rjmp = 35c
; hardware accelerated using Timer0 output compare

; todo: support 1-wire mode by switching to input mode and
; waiting for stop bit before disabling ISR

; PU_T0_SUB = Timer0 prescaled count per bit negated for subi

#define __SFR_OFFSET 0
#include <avr/io.h>

#define CLR_ON_MATCH ((1<<COM0A1) | (1<<WGM01))
#define SET_ON_MATCH ((1<<COM0A1) | (1<<COM0A0) | (1<<WGM01))

; t13 and t85 use TIMSK0/TIMSK but same address
.equiv T0INTMSK, 0x39

TIM0_COMPA_vect:                ; t13 & t84
TIMER0_COMPA_vect:              ; t85
    push r24
    in r24, SREG
    push r24
    lds r24, pu_outchar
    cpi r24, 0
    brne morebits
    out T0INTMSK, r24           ; disable ISR
    rjmp donetx
morebits:
    bst r24, 0                  ; save LSB in T
    lsr r24
    sts pu_outchar, r24
    ;ldi r24, CLR_ON_MATCH
    in r24, TCCR0A
    bld r24, COM0A0             ; set on match when T set
    out TCCR0A, r24
donetx:
    in r24, OCR0A
    subi r24, PU_T0_SUB         ; add time for next bit
    out OCR0A, r24
    pop r24
    out SREG, r24
    pop r24
    reti

.section .noinit, "aw", @nobits
.global pu_outchar
pu_outchar: .zero 1
