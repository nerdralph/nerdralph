/* optimized half-duplex high-speed AVR serial uart
 * Ralph Doncaster 2020 open source MIT license
 *
 * picoUART is accurate to the cycle (+- 0.5 cycle error)
 * 0.64% error at 115.2k/8M and 0.4% error at 115.2k/9.6M
 *
 * define BAUD_RATE before including BBUart.h to change default baud rate 
 *
 * capable of single-pin operation (PU_TX = PU_RX) as follows:
 * connect MCU pin to host RX line, and a 1.5k-4.7k resistor between
 * host RX and TX line.  Note this also gives local echo.
 * 
 * 20200123 version 0.5
 * 20200123 version 0.6  - improve inline asm
 * 20200201 version 0.7  - use push/pull during tx 
 */

#pragma once

#include <avr/io.h>

#ifndef BAUD_RATE
#define BAUD_RATE 115200L               // default baud rate
#endif

#ifndef PU_TX
#define PU_TX B,1
#define PU_RX B,0
#endif

// use static inline functions for type safety
extern inline float BIT_CYCLES() {return F_CPU/(BAUD_RATE*1.0);}

// delay based on cycle count of asm code + 0.5 for rounding
extern inline int PUTXWAIT() {return BIT_CYCLES() - 7 + 0.5;}
extern inline int PURXWAIT() {return BIT_CYCLES() - 5 + 0.5;}

// correct for PURXWAIT skew in PURXSTART calculation
// skew is half of 7 delay intervals between 8 bits
extern inline float PUSKEW() {
    return (BIT_CYCLES() - (int)(BIT_CYCLES() + 0.5)) * 3.5;
}
// Time from falling edge of start bit to sample 1st bit is 1.5 *
// bit-time. Subtract 2 cycles for sbic, 1 for lsr, and PURXWAIT.
// Subtract 1.5 cycles because start bit detection is accurate to
// +-1.5 cycles.  Add 0.5 cycles for int rounding, and add skew.
extern inline int PURXSTART() {
    return (BIT_CYCLES()*1.5 -3 -PURXWAIT() -1 +PUSKEW());
}

// min rx/tx turn-around time in resistor-only 1-wire mode
inline void pu_rxtx_wait()
{
    __builtin_avr_delay_cycles(BIT_CYCLES()*1.5);
}

// I/O register macros
#define BIT(r,b)    (b)
#define PORT(r,b)   (PORT ## r)
#define DDR(r,b)    (DDR ## r)
#define PIN(r,b)    (PIN ## r)
#define bit(io)     BIT(io)
#define port(io)    PORT(io)
#define ddr(io)     DDR(io)
#define pin(io)     PIN(io)

// use up registers so only r25:r24 are free for the compiler 
#define alloc_regs()\
    register int dummy1 asm("r20");\
    asm volatile ("" : "=r" (dummy1));\
    register int dummy2 asm("r26");\
    asm volatile ("" : "=r" (dummy2));\
    register int dummy3 asm("r30");\
    asm volatile ("" : "=r" (dummy3));

#define touch_regs()\
    asm volatile ("" :: "r" (dummy1));\
    asm volatile ("" :: "r" (dummy2));\
    asm volatile ("" :: "r" (dummy3));

__attribute((naked))
void _pu_tx()
{
    alloc_regs();
    register char c asm("r18");
    register char sr asm("r19");
    asm volatile (
    "cbi %[tx_port], %[tx_bit]\n"       // disable pullup
    "cli\n"
    "sbi %[tx_port]-1, %[tx_bit]\n"     // start bit 
    "in r0, %[tx_port]\n"               // save DDR in r0
    "ldi %[sr], 3\n"                    // stop bit & idle state
    "Ltxbit:\n"
    : [c] "+r" (c),
      [sr] "+r" (sr)
    : [tx_port] "I" (_SFR_IO_ADDR(port(PU_TX))),
      [tx_bit] "I" (bit(PU_TX))
    );
    __builtin_avr_delay_cycles(PUTXWAIT());
    // 7 cycle loop
    asm volatile (
    "bst %[c], 0\n"                     // store lsb in T
    "bld r0, %[tx_bit]\n"
    "lsr %[sr]\n"                       // 2-byte shift register
    "ror %[c]\n"                        // shift for next bit
    "out %[tx_port], r0\n"
    "brne Ltxbit\n"
    "cbi %[tx_port]-1, %[tx_bit]\n"     // set to input mode
    "reti\n"                            // return & enable interrupts
    : [c] "+r" (c),
      [sr] "+r" (sr)
    : [tx_port] "I" (_SFR_IO_ADDR(port(PU_TX))),
      [tx_bit] "I" (bit(PU_TX))
    );
    touch_regs();
}

inline void pu_tx(char c)
{
    register char ch asm("r18") = c;
    asm volatile ("%~call %x1" : "+r"(ch) : "i"(_pu_tx) : "r19", "r24", "r25");
}


__attribute((naked))
void _pu_rx()
{
    alloc_regs();
    register char c asm("r18");
    register char dummy4 asm("r19");
    asm volatile (
    // wait for idle state (high)
    "1: sbis %[rx_pin], %[rx_bit]\n"
    "rjmp 1b\n"
    "ldi %[c], 0x80\n"                  // bit shift counter
    "cli\n"
    // wait for start bit (low)
    "1: sbic %[rx_pin], %[rx_bit]\n"
    "rjmp 1b\n"
    : [c] "=d" (c)
    : [rx_pin] "I" (_SFR_IO_ADDR(pin(PU_RX))),
      [rx_bit] "I" (bit(PU_RX))
    );
    __builtin_avr_delay_cycles(PURXSTART());
    asm volatile ("Lrxbit:");
    __builtin_avr_delay_cycles(PURXWAIT());
    // 5 cycle loop 
    asm volatile (
    "lsr %[c]\n" 
    "sbic %[rx_pin], %[rx_bit]\n"
    "ori %[c], 0x80\n"
    "brcc Lrxbit\n"
    "reti\n"
    : [c] "+d" (c)
    : [rx_pin] "I" (_SFR_IO_ADDR(pin(PU_RX))),
      [rx_bit] "I" (bit(PU_RX)),
      "r" (dummy4)
    );
    touch_regs();
}

inline char pu_rx()
{
    register char c asm("r18");
    asm ("%~call %x1" : "=r"(c) : "i"(_pu_rx) : "r24", "r25");
    return c;
}

