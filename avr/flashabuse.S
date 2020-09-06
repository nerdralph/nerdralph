; Ralph Doncaster 2020 public domain software
; repeatedly erase, verify, write, verify flash at OSCCAL 0xFF

#define __SFR_OFFSET 0

#include <avr/io.h>

#ifndef SPMEN
#   define SPMEN SELFPRGEN
#endif

#define scratch r16
#define cnt r17
#define arg1 r18

; LED will flash during testing, stop for failure
#define LED 5

.globl main
main:
    ldi scratch, 0xFF                   ; I feel the need for speed
    sts OSCCAL, scratch
    nop                                 ; let oscillator settle
    sbi DDRB, LED
    sbi PORTB, LED

loop:
    ldi ZH, 0x18                        ; page to abuse = 0x1800
    ldi ZL, 0x00
    rcall erase
    ; verify erase (all 0xFF) 
    ldi arg1, 0xFF
    rcall verify                        ; shold be all 0xFF
    rcall clear
    clr arg1
    rcall verify                        ; shold be all 0x00
    subi cnt, 1
    brhc loop                           ; H set every 16th loop
    sbi PINB, LED                       ; toggle LED
    rjmp loop
fail:
    rjmp .-2                            ; done

; verify page pointed to by Z matches arg1
; jump to fail on falure
verify:
    lpm scratch, Z+
    cp scratch, arg1
    brne fail
    cpi ZL, SPM_PAGESIZE
    brne verify 
    clr ZL
    ret

; set page pointed to by Z to all zero
clear:
    clr r0
    clr r1
    ldi scratch, 1<<SPMEN
fill:
    rcall dospm
    subi ZL, -2                         ; add 2
    cpi ZL, SPM_PAGESIZE
    brne fill 
    clr ZL
    ldi scratch, 1<<SPMEN | 1<<PGWRT
dospm:
    out SPMCSR, scratch
    spm
    ret

; erase page pointed to by Z
erase:
    ldi scratch, 1<<SPMEN | 1<<PGERS
    out SPMCSR, scratch
    spm
    ret
