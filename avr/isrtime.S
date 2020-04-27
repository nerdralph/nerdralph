; AVR interrupt benchmark
; Ralph Doncaster 2020 open source MIT Licence
; connect GPIO to INTPIN
; vary OCR0B value to change PWM low time
; if LED turns on, interrupt latency exceeds pulse time
#define __SFR_OFFSET 0
#include <avr/io.h>

.equ GPIO, 1                            ; GPIO for PWM
.equ INTPIN, 1
.equ LED, 4

.global INT0_vect
INT0_vect:
.global PCINT0_vect
PCINT0_vect:
    ; interrupt triggered by INTPIN low
    ; if high, int latency exceeded pulse width, so turn on LED
    sbic PINB, INTPIN
    sbi PINB, LED
1:  rjmp 1b

.global main
main:
    ; fast PWM, inverting = set OC0B on match
    ldi r18, 1<<COM0B1 | 1<<COM0B0 | 1<<WGM01 | 1<<WGM00
    out TCCR0A, r18
    ldi r18, 6                          ; Tlow = OCR + 1
    out OCR0B, r18                      ; set compare/match
    sbi DDRB, GPIO                      ; output mode
    ; enable timer, no prescaler
    ldi r18, 1<<CS00
    out TCCR0B, r18
1:  sbis PINB, GPIO                     ; wait for high
    rjmp 1b

    ldi r18, 1<<PCIE
    ldi r18, 1<<INT0
    out GIMSK, r18
    ; MCUCR default for INT0 is low level
    ;sbi PCMSK, INTPIN                   ; enable PCINT
    sei

    sbi DDRB, LED
    ldi r18, 1<<SE
    out MCUCR, r18                      ; enable idle sleep
    ;sleep
    ;nop                                 ; 1 clock delay
.Loop:
    rjmp .Loop                          ; loop forever
