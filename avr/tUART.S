; Ralph Doncaster 2020 open source MIT license
; high-speed 8N1 UART - 13 instructions + ret
; ASCII only (bit 7 must be 0)

#define __SFR_OFFSET 0
#include <avr/io.h>

#define TX_GPIO 4

; r24 = char to tx, clobbers r0, r25
ttx:
    ldi r25, 1<<TX_GPIO
    mov r0, r24
    cbi PORTB, TX_GPIO      ; disable pullup
    sbi DDRB, TX_GPIO       ; start bit
    sec                     ; load stop bit into C
    ror r0
    eor r0, r24             ; now 1=toggle, 0 = n/c
morebits:                   ; 5 cycle loop
    brcc 1f                 ; no toggle
0:  out PINB, r25           ; toggle output
1:  lsr r0
    brne morebits
    brcs 0b                 ; toggle for stop bit 
    cbi DDRB, TX_GPIO       ; pullup mode
    ret

.global main
main:
    in r24, OSCCAL
    subi r24, -11           ; tune to 10Mhz
    out OSCCAL, r24
1:  ; loop
    ldi r18, ' '            ; space = ASCII 0x20
    ldi r18, '0'            ; '0' = 0x30
2:  ; write 1 line of ASCII
    mov r24, r18
    rcall ttx
    ;dec r19
    ;brne .-4                ; inter-char delay
    inc r18
    cpi r18, 0x7f
    brne 2b
    ldi r24, '\n'
    rcall ttx
    ldi r18, 38
3:  sbiw r26, 1
    brne 3b                 ; delay 26ms
    dec r18
    brne 3b                 ; 26ms * 38 = 1s
    rjmp 1b                 ; forever

