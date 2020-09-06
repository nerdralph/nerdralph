; Ralph Doncaster 2020 public domain software
; 1M erase, verify, write, verify flash cycles at OSCCAL 0xFF
; then verify cleared page forever
; http://nerdralph.blogspot.com/2020/09/flashing-avrs-at-high-speed.html 

#define __SFR_OFFSET 0

#include <avr/io.h>

#ifndef SPMEN
#   define SPMEN SELFPRGEN
#endif

#define scratch r16
#define cnt r17
#define arg1 r18
#define CYCLES 1000000

; LED will flash during testing, stop for failure
#define LED 5

.globl main
main:
    ldi scratch, 0xFF                   ; I feel the need for speed
    sts OSCCAL, scratch
    nop                                 ; let oscillator settle
    sbi DDRB, LED
    sbi PORTB, LED
    ldi XL, lo8(CYCLES/256)
    ldi XH, hi8(CYCLES/256)
    ldi ZH, 0x18                        ; page to abuse = 0x1800
    ldi ZL, 0x00
loop:
    rcall erase
    ; verify erase (all 0xFF) 
    ldi arg1, 0xFF
    rcall verify                        ; shold be all 0xFF
    rcall clear
    clr arg1
    rcall verify                        ; shold be all 0x00
    subi cnt, 1
    brhc loop                           ; H set every 16th loop
    sbi PINB, LED                       ; fast toggle LED
    brcc loop
    sbiw XL, 1
    brne loop
    ; now repeatedly verify
1:  rcall verify
    adiw XL, 2
    brne 1b
    sbi PINB, LED                       ; slow toggle LED
    rjmp 1b
fail:
    rjmp fail                           ; done

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
