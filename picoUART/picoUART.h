/* optimized half-duplex high-speed AVR serial uart
 * Ralph Doncaster 2020 open source MIT license
 * open-drain UART requires external pullup between 4.7k and 10k
 * if remote Rx line does not already have a pullup
 *
 * picoUART is accurate to the cycle (+- 0.5 cycle error)
 * 0.64% error at 115.2k/8M and 0.4% error at 115.2k/9.6M
 *
 * define BAUD_RATE before including BBUart.h to change default baud rate 
 *
 * capable of single-pin operation (PU_TX = PU_RX)
 * 
 * 20200123 version 0.5
 */

#pragma once

#include <avr/io.h>

#ifndef BAUD_RATE
#define BAUD_RATE 115200L               // default baud rate
#endif

#define BIT_CYCLES (F_CPU/(BAUD_RATE*1.0)) 
// delay based on cycle count of asm code + 0.5 for frounding
#define PUTXWAIT  (BIT_CYCLES - 7 + 0.5)
#define PURXWAIT  (BIT_CYCLES - 5 + 0.5)
// correct for PURXWAIT skew in PURXSTART calculation
// skew is half of 7 delay intervals between 8 bits
#define PUSKEW ((PURXWAIT - (int)(PURXWAIT + 0.5)) * 3.5 + 0.5)
#define PURXSTART ((BIT_CYCLES)/2 - 2.5 - PUSKEW)

// I/O register macros
#define BIT(r,b)    (b)
#define PORT(r,b)   (PORT ## r)
#define DDR(r,b)    (DDR ## r)
#define PIN(r,b)    (PIN ## r)
#define bit(io)     BIT(io)
#define port(io)    PORT(io)
#define ddr(io)     DDR(io)
#define pin(io)     PIN(io)

#define PU_TX B,1
#define PU_RX B,0

__attribute((naked))
void pu_tx(char c)
{
    volatile char bitcnt = 10;          // start + 8bit + stop = 10 bits
    asm volatile (
    "cli\n"
    "sbi %[tx_ddr], %[tx_pin]\n"        // start bit 
    "in r0, %[tx_ddr]\n"                // save DDR in r0
    "com %[c]\n"                        // invert for open drain
    "Ltxbit:\n"
    : [c] "+r" (c)
    : [tx_ddr] "I" (_SFR_IO_ADDR(ddr(PU_TX))),
      [tx_pin] "I" (bit(PU_TX)),
      "r" (bitcnt)                      // force bitcnt init
    );
    __builtin_avr_delay_cycles(PUTXWAIT);
    // 7 cycle loop
    asm volatile (
    "bst %[c], 0\n"                     // store lsb in T
    "bld r0, %[tx_pin]\n"
    "lsr %[c]\n"                        // shift for next bit
    "dec %[bitcnt]\n"
    "out %[tx_ddr], r0\n"
    "brne Ltxbit\n"
    "reti\n"                            // return & enable interrupts
    : [c] "+r" (c),
      [bitcnt] "+r" (bitcnt)
    : [tx_ddr] "I" (_SFR_IO_ADDR(ddr(PU_TX))),
      [tx_pin] "I" (bit(PU_TX))
    );
}


__attribute((naked))
char pu_rx()
{
    char c;
    asm volatile (
    // wait for idle state (high)
    "1: sbis %[rx_pin], %[rx_bit]\n"
    "rjmp 1b\n"
    "ldi %[c], 0x80\n"                  // bit shift counter
    "cli\n"
    // wait for start bit (low)
    "1: sbic %[rx_pin], %[rx_bit]\n"
    "rjmp 1b\n"
    //"sbi %[rx_pin], 3\n"                // debug
    : [c] "=d" (c)
    : [rx_pin] "I" (_SFR_IO_ADDR(pin(PU_RX))),
      [rx_bit] "I" (bit(PU_RX))
    );
    __builtin_avr_delay_cycles(PURXSTART);
    asm volatile ("Lrxbit:");
    __builtin_avr_delay_cycles(PURXWAIT);
    // 5 cycle loop 
    asm volatile (
    "lsr %[c]\n" 
    "sbic %[rx_pin], %[rx_bit]\n"
    "ori %[c], 0x80\n"
    //"sbi %[rx_pin], 3\n"                // debug
    "brcc Lrxbit\n"
    "reti\n"
    : [c] "+d" (c)
    : [rx_pin] "I" (_SFR_IO_ADDR(pin(PU_RX))),
      [rx_bit] "I" (bit(PU_RX))
    );
    return c;
}

