/* optimized half-duplex serial uart implementation
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 */

extern void TxByte(unsigned char);
extern unsigned char RxByte();

#define STR1(x) #x
#define STR(x) STR1(x)

#ifndef BAUD_RATE
// default baud rate
#define BAUD_RATE 57600
#endif

#define DIVIDE_ROUNDED(NUMERATOR, DIVISOR) ((((2*(NUMERATOR))/(DIVISOR))+1)/2)

// txbit takes 3*RXDELAY + 15 cycles
#define BIT_CYCLES DIVIDE_ROUNDED(F_CPU,BAUD_RATE) 
#define TXDELAYCOUNT DIVIDE_ROUNDED(BIT_CYCLES - 15, 3)

#define RXSTART_CYCLES DIVIDE_ROUNDED(3*F_CPU,2*BAUD_RATE) 
// 1st bit sampled 3*RXDELAY + 11 cycles after start bit begins
#define RXSTARTCOUNT DIVIDE_ROUNDED(RXSTART_CYCLES - 11, 3)
// rxbit takes 3*RXDELAY + 12 cycles
#define RXDELAYCOUNT DIVIDE_ROUNDED(BIT_CYCLES - 12, 3)

#if ( RXSTART_CYCLES > 255 )
#error baud rate too low - must be >= 19200 @ 8Mhz, 2400 @ 1Mhz
#endif

asm(".global TXDELAY" );
asm(".equ TXDELAY, " STR(TXDELAYCOUNT) );
asm(".global RXSTART" );
asm(".equ RXSTART, " STR(RXSTARTCOUNT) );
asm(".global RXDELAY" );
asm(".equ RXDELAY, " STR(RXDELAYCOUNT) );
