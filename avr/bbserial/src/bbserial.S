/* asynchronous serial output for AVR MCUs
 * supports runtime baud rate setting  
 * (c) Ralph Doncaster 2021  - MIT Licence
 */

/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

#include "config.h"

#define UART_DDR (UART_Port-1)

.lcomm CyclesPerBit, 2
.global CyclesPerBit

#define delayArg XL
#define bitDelayLo ZL
#define bitDelayHi ZH

; output byte contained in r24
.global WriteB
WriteB:
    lds bitDelayLo, CyclesPerBit
    lds bitDelayHi, CyclesPerBit+1
    sbiw bitDelayLo, 6                  ; subtract overhead
    andi bitDelayLo, 0xFC               ; round to nearest 4c
    cli
    cbi UART_Port, UART_Tx              ; disable pullup
    sbi UART_DDR, UART_Tx               ; output mode for start bit
    in r0, UART_Port
    ldi r25, 3                          ; stop bit & idle state
TxLoop:
    ; 7 cycle loop + 4 * delayArg
    movw delayArg, bitDelayLo
TxDelay:
    ; 4 cycles per delay loop
    sbiw delayArg, 4
    brne TxDelay
    bst r24, 0                          ; store lsb in T
    bld r0, UART_Tx
    lsr r25
    ror r24                             ; 2-byte shift register
    out UART_Port, r0
    brne TxLoop
    cbi UART_DDR, UART_Tx               ; revert to input mode
    reti                                ; return & enable interrupts

