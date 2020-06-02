; (c) Ralph Doncaster 2020
; hardware accelerated UART using Timer0 output compare

#define __SFR_OFFSET 0
#include <avr/io.h>
#include "wgmuart.h"

; CLR_ON_MATCH is (1<<COM0A1)
; SET_ON_MATCH is ((1<<COM0A1) | (1<<COM0A0))

.macro ADDI Rd, K
    subi \Rd, lo8(-(\K))
.endm

.macro GLABEL name
    .global \name
    \name:
.endm

; t13 and t85 use TIMSK0/TIMSK but same address
.equiv T0INTMSK, 0x39

; tx bit ISR 25c incl reti
compa_ISR_off:
    in r24, T0INTMSK
    cbr r24, 1<<OCIE0A
    out T0INTMSK, r24               ; disable ISR
    rjmp setOCR
GLABEL TIM0_COMPA_vect              ; t13
GLABEL TIMER0_COMPA_vect            ; t85
    sei                             ; allow other ISRs to run
    in r2, SREG
    push r24
    lds r24, wgm_txdata
    cpi r24, 0
    breq compa_ISR_off              ; 10c
    bst r24, 0                      ; save LSB in T
    lsr r24
    sts wgm_txdata, r24
    in r24, TCCR0A
    bld r24, COM0A0                 ; set on match when T set
    out TCCR0A, r24
setOCR:
    in r24, OCR0A
    ADDI r24, TICKS_PER_BIT
    out OCR0A, r24                  ; 20c, set time for next bit
GLABEL epilogue                     ; shared ISR epilogue
    pop r24
    out SREG, r2
reti

; rx bit ISR 23c incl reti
; bit sampled 8 cycles into ISR
GLABEL TIM0_COMPB_vect              ; t13
GLABEL TIMER0_COMPB_vect            ; t85
    in r22, SREG
    push r24
    lds r24, wgm_rxdata
    bst r24, 0                      ; save LSB in T
    lsr r24
    sbic PINB, WGMRXBIT             ; 10c
    ori r24, 0x80                   ; rx bit is a 1
    sts wgm_rxdata, r24
    in r24, OCR0B
    ADDI r24, TICKS_PER_BIT
    out OCR0B, r24                  ; set time for next bit
    brtc epilogue                   ; still more bits to rx
    in r24, T0INTMSK
    cbr r24, 1<<OCIE0B
    out T0INTMSK, r24               ; disable ISR
    sbi PORTB, WGMRXBIT             ; flag Rx complete
    rjmp epilogue

;.section .noinit, "aw", @nobits
;.global wgm_txdata
;wgm_txdata: .zero 1
;.global wgm_rxdata
;wgm_rxdata: .zero 1
