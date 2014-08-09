/* optimized half-duplex high-speed serial uart implementation
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 * soft UART has only 0.8% timing error at default 115200 baud rate @8Mhz
 * and 2.1% timing error at 230400.
 * To use UART, include BBUart.h in your C program, compile BBUart.S,
 * and link BBUart.o with your program.
 *
 * define BAUD_RATE before including BBUart.h to change default baud rate 
 */

#ifndef F_CPU
#error F_CPU undefined
#endif

#ifdef __cplusplus
extern "C" {
#endif
    void TxByte(unsigned char);
    unsigned char RxByte();
#ifdef __cplusplus
}
#endif

#define STR1(x) #x
#define STR(x) STR1(x)

#ifndef BAUD_RATE
// default baud rate
#define BAUD_RATE 115200 
#endif

#define DIVIDE_ROUNDED(NUMERATOR, DIVISOR) ((((2*(NUMERATOR))/(DIVISOR))+1)/2)

// txbit takes 3*RXDELAY + 15 cycles
#define BIT_CYCLES DIVIDE_ROUNDED(F_CPU,BAUD_RATE) 
#define TXDELAYCOUNT DIVIDE_ROUNDED(BIT_CYCLES - 7, 3)

#define RXSTART_CYCLES DIVIDE_ROUNDED(3*F_CPU,2*BAUD_RATE) 
// 1st bit sampled 3*RXDELAY + 11 cycles after start bit begins
#define RXSTARTCOUNT DIVIDE_ROUNDED(RXSTART_CYCLES - 13, 3)
// rxbit takes 3*RXDELAY + 12 cycles
#define RXDELAYCOUNT DIVIDE_ROUNDED(BIT_CYCLES - 13, 3)

#if ( RXSTARTCOUNT > 255 )
#error baud rate too low - must be >= 19200 @ 8Mhz, 2400 @ 1Mhz
#endif

asm(".global TXDELAY" );
asm(".global RXSTART" );
asm(".global RXDELAY" );

// dummy function defines no code
// hack to define absolute linker symbols using C macro calculations
static void dummy() __attribute__ ((naked));
static void dummy() __attribute__ ((used));
static void dummy(){
asm (
    ".equ TXDELAY, %[txdcount]\n"
    ::[txdcount] "M" (TXDELAYCOUNT)
    );
asm (
    ".equ RXSTART, %[rxscount]\n"
    ::[rxscount] "M" (RXSTARTCOUNT)
    );
asm (
    ".equ RXDELAY, %[rxdcount]\n"
    ::[rxdcount] "M" (RXDELAYCOUNT)
    );
}

