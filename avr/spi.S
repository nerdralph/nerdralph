/* AVR software SPI in optimized assembler
 * @author: Ralph Doncaster 2015
 */

/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

#define SPIPORT PORTB
#define MOSI 4
#define CLK 5
#define mosipinmask (1<<MOSI)
#define clkpinmask (1<<CLK)

.global spiByte
spiByte:
    in r18, SPIPORT     ; save port state
    andi r18, ~(mosipinmask | clkpinmask)  
    ldi r20, mosipinmask
    ldi r19, clkpinmask
    lsl r24
    ori r24, 0x01       ; 9th bit marks end of byte
spiBit:
    out SPIPORT, r18
    brcc zeroBit
    out SPIPORT-2, r20  ; PORT address -2 is PIN
    lsl r24
    out SPIPORT-2, r19  ; clk hi
    brne spiBit
    ret

.global spiByte
spiFast:
    in r25, SPIPORT     ; save port state
    andi r25, ~clkpinmask
    ldi r19, clkpinmask
halfByte:
    bst r24, 7 
    bld r25, MOSI
    out SPIPORT, r25    ; clk low + data
    out SPIPORT-2, r19  ; clk hi
    bst r24, 6 
    bld r25, MOSI
    out SPIPORT, r25    ; clk low + data
    out SPIPORT-2, r19  ; clk hi
    bst r24, 5 
    bld r25, MOSI
    out SPIPORT, r25    ; clk low + data
    out SPIPORT-2, r19  ; clk hi
    bst r24, 4 
    bld r25, MOSI
    out SPIPORT, r25    ; clk low + data
    out SPIPORT-2, r19  ; clk hi
    swap r24
    eor r1, r19         ; r1 is zero reg
    brne halfByte
    ret
