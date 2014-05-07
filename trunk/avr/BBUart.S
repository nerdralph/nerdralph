/* optimized half-duplex serial uart implementation - 27 instructions
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 */

/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

#ifdef PORTD
#define UART_Port PORTD
#else
#define UART_Port PORTB
#endif
#define UART_Tx 1
#define UART_Rx 0

#define delayArg r22

.extern TXDELAY
.extern RXSTART
.extern RXDELAY

; transmit byte contained in r24
; AVR305 has 1 cycle of jitter per bit, this has none
.global TxByte
TxByte:
	sbi UART_Port-1, UART_Tx		; set Tx line to output
	cbi UART_Port, UART_Tx			; start bit
	in r0, UART_Port
	ldi r25, 3						; stop bit & idle state
TxLoop:
	; 8 cycle loop + delay = 7 + 3*DelayArg
	ldi delayArg, TXDELAY
TxDelay:
    dec delayArg
	brne TxDelay
	bst r24, 0						; store lsb in T
	bld r0, UART_Tx
	lsr r25
	ror r24							; 2-byte shift register
	out UART_Port, r0
	brne TxLoop
	ret

; receive byte into r24
.global RxByte
RxByte:
	sbic UART_Port-2, UART_Rx		; wait for start edge
	rjmp RxByte
	ldi r24, 0x80					; bit shift counter
	ldi delayArg, RXSTART			; 1.5 bit delay
RxBit:
	; 7 cycle loop + delay = 7 + 6 + 3*DelayArg
	rcall Delay3Cycle				; delay and clear carry
	ldi delayArg, RXDELAY 
	lsr r24
	sbic UART_Port-2, UART_Rx
	ori r24, 0x80
    nop                             ; match 7-cycle Tx loop
	brcc RxBit
	; fall into delay for stop bit

; delay (3 cycle * delayArg) -1 + 4 cycles (ret instruction)
Delay3Cycle:
	dec delayArg
	brne Delay3Cycle
	ret
