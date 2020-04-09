; Ralph Doncaster 2020 open source MIT license
; 8N1 UART, 2 cycles per bit

#define __SFR_OFFSET 0
#include <avr/io.h>

#define TX_GPIO 0

; 25 instructions including ret
; r24 = char to tx, clobbers r0, r25
ttx:
    ldi r25, 1<<TX_GPIO
    mov r0, r24
    lsl r0
    eor r0, r24             ; now 1=toggle, 0 = n/c
    cbi PORTB, TX_GPIO      ; disable pullup
    sbi DDRB, TX_GPIO       ; start bit
    sbrc r0, 0
    out PINB, r25           ; bit 0
    sbrc r0, 1
    out PINB, r25           ; bit 1
    sbrc r0, 2
    out PINB, r25           ; bit 2
    sbrc r0, 3
    out PINB, r25           ; bit 3
    sbrc r0, 4
    out PINB, r25           ; bit 4
    sbrc r0, 5
    out PINB, r25           ; bit 5
    sbrc r0, 6
    out PINB, r25           ; bit 6
    sbrc r0, 7
    out PINB, r25           ; bit 7
    sbi PORTB, TX_GPIO      ; stop bit
    cbi DDRB, TX_GPIO       ; pullup mode
    ret

.global main
main:
    in r24, OSCCAL
    subi r24, -11           ; tune oscillator
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

