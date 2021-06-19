/* AVR software SPI in optimized assembler
 * author: Ralph Doncaster 2015
 * open source MIT licence
 * functions follow avr-libc calling convention (r24)
 */

/* needed for <avr/io.h> to give io constant addresses */
#define __SFR_OFFSET 0
#include <avr/io.h>

#define tmpreg r0
#define SPIPORT PORTB
#define MISO 3
#define MOSI 4
#define CLK 5
#define mosipinmask (1<<MOSI)
#define clkpinmask (1<<CLK)

.global spiOut
spiOut:
    in tmpreg, SPIPORT     ; save port state
    ldi r20, mosipinmask
    ldi r19, clkpinmask
    lsl r24
    ori r24, 0x01       ; 9th bit marks end of byte
spiBit:
    brcc zeroBit
    out SPIPORT-2, r20  ; PORT address -2 is PIN
zeroBit:
    out SPIPORT-2, r19  ; clk hi
    lsl r24
    out SPIPORT, tmpreg ; reset port state
    brne spiBit
    ret


; receive-only SPI master
.global spiIn
spiIn:
    ldi r24, 0x01
    clc
    ldi r25, clkpinmask
bitIn:
    out SPIPORT-2, r25  ; clk hi
    sbic SPIPORT-2, MISO
    sec
    rol r24             ; shift in data
    out SPIPORT-2, r25  ; clk hi
    brcc bitIn
    ret


; receive-only SPI slave
.global spiSIn
spiSIn:
    ldi r24, 0x01
    clc
bitSIn:
    sbis SPIPORT-2, CLK
    rjmp bitSIn         ; wait for clk hi
    sbic SPIPORT-2, MOSI
    sec
    rol r24             ; shift in data
waitClk:
    sbis SPIPORT-2, CLK
    rjmp waitClk         ; wait for clk lo
    brcc bitSIn
    ret


.global spiFast
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
