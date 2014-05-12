/* sniffer - captures D-/D+ on PB0/PB1 at 2.4msps transmits data
 * 6mbps 8N1 serial UART using synchronous mode - requires 12Mhz clock
 * core asm routine - requires UART configuration before calling capture2()
 * @author: Ralph Doncaster 2014
 */

/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

#define tmp r16
#define sample r17
#define nop2 rjmp .+0

.global capture2
; 20-cycle capture/xmit loop
capture2:
    in  sample, PINB
    out UDR, tmp
    andi sample, 0x03
    lsl sample
    lsl sample
    in  tmp, PINB
    andi tmp, 0x03
    or sample, tmp
    lsl sample
    lsl sample
    in  tmp, PINB
    andi tmp, 0x03
    or sample, tmp
    lsl sample
    lsl sample
    in  tmp, PINB
    andi tmp, 0x03
    or tmp, sample
    rjmp capture2


/* sniffer - captures PORTB at 600ksps transmits data
 * 6mbps 8N1 serial UART using synchronous mode - requires 12Mhz clock
 * core asm routine - requires UART configuration before calling capture8()
 * @author: Ralph Doncaster 2014
 */

.global capture8
; 20-cycle capture/xmit loop
capture8:
    in  sample, PINB
    rcall sevenCycle
    rcall sevenCycle
    nop2
    out UDR, sample
    rjmp capture8

sevenCycle:
    ret
