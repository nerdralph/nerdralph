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

#define BIT_CYCLES (F_CPU/BAUD_RATE) 
// delay based on cycle count of asm code + 0.5 for frounding
#define PUTXWAIT  (BIT_CYCLES - 7 + 0.5)
// could correct for RXDELAYCOUNT error in RXSTARTCOUNT calculation
#define PURXSTART ((BIT_CYCLES * 1.5) - 13 + 0.5)
#define PURXWAIT  ((BIT_CYCLES - 13 + 0.5)

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
    char bitcnt = 10;                   // start + 8bit + stop = 10 bits
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

/*
; receive byte into r24
.global RxByte
RxByte:
    sbic UART_Port-2, UART_Rx           ; wait for start edge
    rjmp RxByte
    ldi r24, 0x80                       ; bit shift counter
    ldi delayArg, RXSTART               ; 1.5 bit delay
RxBit:
    ; 7 cycle loop + delay = 7 + 6 + 3*DelayArg
    rcall Delay3Cycle                   ; delay and clear carry
    ldi delayArg, RXDELAY 
    lsr r24
    sbic UART_Port-2, UART_Rx
    ori r24, 0x80
    nop                                 ; match 7-cycle Tx loop
    brcc RxBit
    ; fall into delay for stop bit

; delay (3 cycle * delayArg) -1 + 4 cycles (ret instruction)
Delay3Cycle:
    dec delayArg
    brne Delay3Cycle
    ret
*/
