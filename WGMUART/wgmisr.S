; (c) Ralph Doncaster 2020
; Non-commercial use and modification of this code is permitted.
; Any re-distribution must include this notice.

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

; tx bit ISR 29c incl reti
compa_ISR_off:
    cli
    in r24, TIMSK
    cbr r24, 1<<OCIE0A
    sei
    out TIMSK, r24                  ; disable ISR
    rjmp setOCR
GLABEL TIM0_COMPA_vect              ; t13 & t84
GLABEL TIMER0_COMPA_vect            ; t85
    sei                             ; allow other ISRs to run
    push r24
    in r24, SREG
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
    pop r24
    out SREG, r24
    pop r24
reti

; rx bit ISR 23c incl reti
; bit sampled 8 cycles into ISR
GLABEL TIM0_COMPB_vect              ; t13 & t84
GLABEL TIMER0_COMPB_vect            ; t85
    in r22, SREG
    push r24
    lds r24, wgm_rxdata0
    bst r24, 0                      ; save LSB in T
    lsr r24
    sbic PINB, WGMRXBIT             ; 10c
    ori r24, 0x80                   ; rx bit is a 1
    sts wgm_rxdata0, r24
    in r24, OCR0B
    ADDI r24, TICKS_PER_BIT
    out OCR0B, r24                  ; set time for next bit
    brtc epilogue                   ; still more bits to rx
    in r24, TIMSK
    cbr r24, 1<<OCIE0B
    out TIMSK, r24                  ; disable T0 ISR
    sbis WGMRXPORT, WGMRXBIT        ; rxdata1 full?
    rjmp copyrxdata
    ; OC0A_BIT in OC0A_PORT used to flag rxdata0 full
    sbi OC0A_PORT, OC0A_BIT
    rjmp epilogue
copyrxdata:
    lds r24, wgm_rxdata0
    sts wgm_rxdata1, r24
    sbi WGMRXPORT, WGMRXBIT         ; flag Rx data available
1:  sbis WGMRXPIN, WGMRXBIT         ; wait for stop bit
    rjmp 1b
    ldi r24, 1<<WGMRXBIT
    out PCMSK, r24                  ; enable PCINT for next byte
GLABEL epilogue                     ; shared ISR epilogue
    pop r24
    out SREG, r2
reti

