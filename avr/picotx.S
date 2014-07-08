/* tiny tx-only UART - small but clobbers other port bits 
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

/* only works with port pin 0 */
#define UART_Tx 0

#define delayArg r22

.extern TXDELAY

; transmit byte contained in r24
.global TxByte
TxByte:
	cbi UART_Port, UART_Tx			; start bit
	sbi UART_Port-1, UART_Tx		; set Tx line to output
	ldi r25, 3						; stop bit & idle state
TxLoop:
	; 6 cycle loop + delay = 5 + 3*DelayArg
	ldi delayArg, TXDELAY
TxDelay:
    dec delayArg
	brne TxDelay
	lsr r25
    out UART_Port, r24
	ror r24							; 2-byte shift register
	brne TxLoop
	ret

