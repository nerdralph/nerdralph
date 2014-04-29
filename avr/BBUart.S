/* optimized half-duplex serial uart implementation - 27 instructions
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 */

#ifndef F_CPU
#error F_CPU undefined
#endif

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
	; 9 cycle loop + delay
	ldi delayArg, TXDELAY
	rcall Delay3Cycle				; delay + 3 cycles for rcall
	bst r24, 0						; store lsb in T
	bld r0, UART_Tx
	lsr r25
	ror r24							; 2-byte shift register
	out UART_Port, r0
    nop                             ; make loop a multiple of 3 cycles
	brne TxLoop
	ret

; receive byte into r24
.global RxByte
RxByte:
	ldi r24, 0x80					; bit shift counter
WaitStart:
	sbic UART_Port-2, UART_Rx		; wait for start edge
	rjmp WaitStart
	ldi delayArg, RXSTART			; 1.5 bit delay
RxBit:
	rcall Delay3Cycle				; delay and clear carry
	; 6 cycle loop
	ldi delayArg, RXDELAY 
	sbic UART_Port-2, UART_Rx
	sec
	ror r24
	brcc RxBit
	; fall into delay for stop bit

; delay (3 cycle * delayArg) -1 + 4 cycles (ret instruction)
; also clears carry (subi instead of dec) to save 1 instr in RxBit
Delay3Cycle:
	subi delayArg, 1
	brne Delay3Cycle
	ret
