#define __SFR_OFFSET 0
#include <avr/io.h>

; define UART_bit - has to be shared rx/tx
#define UART_bit 3
#define UART_Port PORTB
#define bitcnt r25
; 115.2 @ 8M
#define bdelay r18
#define BITDELAY 18

; 23 instructions total vs 27 for BBUart

; timing loop 1: tx LSB, sample start bit ...
;  loop 9: tx stop bit, sample MSB
; open drain, 1 == low, 0 = high
utxstart:
    sbi UART_Port-1, UART_bit           ; start bit
    ldi bdelay, BITDELAY 
urxtx:
    in r0, UART_Port-1                  ; save DDR
    com r24
    ldi bitcnt, 9
ODLoop:                                 ; 16 cycles/bit + 3*delay
    rcall delay3cycle
    bst r24, 0                          ; store lsb in T
    bld r0, UART_bit
    lsr r24
    sbic UART_Port-2, UART_bit          ; sample rx bit
    ori r24, 0x80
    out UART_Port-1, r0
    ldi bdelay, BITDELAY 
    dec bitcnt
    brne ODLoop
    ret

; uart tx
utx:
    rcall utxstart
    ; fall into delay for stop bit
delay3cycle:
    dec bdelay
    brne delay3cycle
    ret

urx:
    ldi r24, 0xFF
    ldi bdelay, 1
    rjmp urxtx

.global main
main:
    ldi r24, 'H'
    rcall utx
    ldi r24, 'i'
    rcall utx
    ldi r24, '\n'
    rcall utx
1:  ; wait 2^16 cycles
    sbiw r24, 1
    brne 1b
    dec r26
    brne 1b
    rjmp main

