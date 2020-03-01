/* debug uart
 * Ralph Doncaster 2020 open source MIT license
 */

#include <avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PU_TX
#define PU_TX B,1
#endif

#ifndef DTXWAIT
#define DTXWAIT 0 
#endif

// I/O register macros
#define BIT(r,b)    (b)
#define PORT(r,b)   (PORT ## r)
#define DDR(r,b)    (DDR ## r)
#define PIN(r,b)    (PIN ## r)
#define bit(io)     BIT(io)
#define port(io)    PORT(io)
#define ddr(io)     DDR(io)
#define pin(io)     PIN(io)

extern inline void _dbgtx()
{
    register char c asm("r18");
    asm volatile (
    "cbi %[tx_port], %[tx_bit]\n"       // disable pullup
    "sbi %[tx_port]-1, %[tx_bit]\n"     // start bit 
    "in r0, %[tx_port]\n"               // save port state
    "sec\n"                             // hold stop bit in C
    "1:\n"                              // tx bit loop
#if DTXWAIT & 1
    "nop\n"                             // wait 1 cycle
#endif
#if DTXWAIT & 2
    "rjmp .\n"                          // wait 2 cycles
#endif
    // 7 cycle loop
    "bst %[c], 0\n"                     // store lsb in T
    "bld r0, %[tx_bit]\n"
    "ror %[c]\n"                        // shift for next bit
    "clc\n"
    "out %[tx_port], r0\n"
    "brne 1b\n"
    "cbi %[tx_port]-1, %[tx_bit]\n"     // set to input mode
    : [c] "+r" (c)
    : [tx_port] "I" (_SFR_IO_ADDR(port(PU_TX))),
      [tx_bit] "I" (bit(PU_TX))
    );
}

inline void dbgtx(char ch)
{
    register char c asm("r18") = ch;
    asm volatile ( "%~call %x1" : "+r"(c) : "i"(_dbgtx) );
}

extern inline void _dprint()
{
    asm volatile (
    "%~call %x0\n"
    ".global dprintz\n"
    "dprintz:\n"
    "lpm r18, Z+\n"                     // read next char
    "tst r18\n"
    "brne _dprint\n"
    :
    : "i" (_dbgtx)
    : "r18"
    );
}

// print string in flash
inline void dprint(const char* s)
{
    asm volatile (
    //"%~call %x1\n"
    "%~call dprintz\n"
    : "+z" (s)
    : "i" (_dprint)
    : "r18"
    );
}

// print r19 as hex, clobbers r18
extern inline void _dprinthex()
{
    asm volatile (
    "mov r18, r19\n"
    "swap r18\n"
    "rcall nibbletohex\n"               //convert hi digit
    "mov r18, r19\n"
    // fall into nibbletohex to convert lo digit
"nibbletohex:\n"
    "andi r18, 0x0F\n"
    "cpi r18, 10\n"
    "brlo under10\n"
    "subi r18, -('A'-'0')\n"
"under10:\n"
    "subi r18, -'0'\n"
    "%~call %x0\n"
    :
    : "i" (_dbgtx)
    : "r18", "r19"
    );
}

inline void dprinthex(uint8_t val)
{
    register char c asm("r19") = val;
    asm volatile (
    "%~call %x1\n"
    : 
    : "r" (c), "i" (_dprinthex)
    : "r18"
    );
}

#ifdef __cplusplus
} // extern "C"
#endif
