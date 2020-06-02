// (c) Ralph Doncaster 2020 - ralphdoncaster at gmail 
// Non-commercial use and modification of this code is permitted.
// Any re-distribution must include this notice.
//
// AVR WGM full-duplex UART using timer/counter0
// Maximum baud rate is F_CPU/70, i.e. 114kbps for 8Mhz
// Minimum baud rate is F_CPU/1350, i.e. 5900bps for 8Mhz
// 
// 20200526 v0.1.0 working version in C
// 20200531 v0.1.1 COMPA & COMPB ISR in asm

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "wgmuart.h"

__attribute(( section(".noinit") )) uint8_t wgm_txdata;
__attribute(( section(".noinit") )) uint8_t wgm_rxdata;

#ifndef OC0A_BIT
#define OC0A_BIT PB0
#define OC0A_DDR DDRB
#endif

#define BAUD_RATE 115200

const int LED = 4;

#define DIVIDE_ROUNDED(NUMBER, DIVISOR) ((((2*(NUMBER))/(DIVISOR))+1)/2)
#define PRESCALER 8L
#define TICKS_PER_BIT DIVIDE_ROUNDED(F_CPU, PRESCALER * BAUD_RATE)

#if (TICKS_PER_BIT > 171)
#error badurate too low
#endif
#if (TICKS_PER_BIT * PRESCALER < 69)
#error badurate too high
#endif

// timer normal waveform generation mode
#define CLEAR_ON_MATCH (1<<COM0A1)
#define SET_ON_MATCH ((1<<COM0A1) | (1<<COM0A0))

#ifdef DEBUG
#define debug() PINB = 1<<PB2
#else
#define debug() {}
#endif

#ifdef TIMSK0
#define TIMSK TIMSK0
#define TIFR TIFR0
#endif

// Rx start ISR 27c incl reti
// TCNT0 sampled 8c into ISR + 6c PCINT latency + 2c rjmp = 16c
ISR(PCINT0_vect, ISR_NAKED)
{
    debug();
    asm(
    "in r2, __SREG__    \n"             // r2 fixed register
    "push r24           \n"             // ISR only uses r24
    );
    // 32 cycle PCINT + COMPB ISR overhead from asm cycle count
    uint8_t isr_overhead_ticks = DIVIDE_ROUNDED(32, PRESCALER);
    uint8_t first_bit_ticks = (TICKS_PER_BIT * 1.5) - isr_overhead_ticks;
    PCMSK &= ~(1<<WGMRXBIT);            // turn off PCINT
    OCR0B = TCNT0 + first_bit_ticks;
    wgm_rxdata = 0x80;                  // setup bit shift counter
    TIFR = 1<<OCF0B;                    // clear OC0B flag
    TIMSK |= 1<<OCIE0B;                 // enable Rx timer ISR
    asm( "rjmp epilogue" );
}

// returns true when there is data to read
uint8_t rx_data_ready()
{
    // data is ready if PCINT & TIM0_COMPB disabled
    //return !(PCMSK & 1<<WGMRXBIT) && !(TIMSK & 1<<OCIE0B);
    return (PORTB & 1<<WGMRXBIT);
}

uint8_t rx_read()
{
    uint8_t data = wgm_rxdata;
    // wait for stop bit/idle
    loop_until_bit_is_set(PINB, WGMRXBIT);
    PCMSK |= 1<<WGMRXBIT;               // enable Rx ISR
    PORTB &= ~(1<<WGMRXBIT);            // clear Rx complete flag
    return data;
}

void UARTsetup() {
    asm(".global TICKS_PER_BIT");
    asm(".equiv TICKS_PER_BIT, %0" :: "M" (TICKS_PER_BIT) );
    // Tx setup
    TCCR0A = SET_ON_MATCH;
    // start timer0 /8 prescaler, force compare
    TCCR0B = 1<<CS01 | 1<<FOC0A;
    OC0A_DDR |= 1<<OC0A_BIT;

    // Rx setup
    PCMSK |= 1<<WGMRXBIT;
    GIMSK = 1<<PCIE;
    sei();
}

void write(uint8_t c) {
    while (TIMSK & 1<<OCIE0A);         // wait for last tx to finish
    // OC0A still running in SET_ON_MATCH mode during idle
    // stop bit time is finished if OCF0A set
    while ((TIFR & 1<<OCF0A) == 0);
    // todo: disable/enable interrupts
    TIFR = 1<<OCF0A;
    TCCR0A = CLEAR_ON_MATCH;            // setup for start bit
    // timer0 /8 prescaler, force compare
    TCCR0B = 1<<CS01 | 1<<FOC0A;
    OCR0A = TCNT0 + TICKS_PER_BIT;

    // if first bit is set, switch to SET_ON_MATCH
    if (c & 0x01)
        TCCR0A = SET_ON_MATCH;

    // high bit flag for end of byte
    wgm_txdata = (c>>1) | 0x80;
    TIMSK |= 1<<OCIE0A;
}

void prints_P(const __flash char* s)
{
    while (*s) write(*s++);
}

void main() {
    UARTsetup();
    DDRB |= 1<<LED;
    prints_P(PSTR("\nwgmUART echo\n"));
    const unsigned ovf_per_sec = F_CPU / PRESCALER / 256;
    unsigned overflows = 0;
    while (1) {
        if ( rx_data_ready() ) {
            write( rx_read() ); 
        }
        if (TIFR & 1<<TOV0) {
            TIFR = 1<<TOV0;       // clear overflow flag
            overflows++;
        }
        // write a dot and toggle LED every second
        if ( overflows == (ovf_per_sec) ) {
            overflows = 0;
            write('.');
            PINB |= 1<<LED;
        }
    }
}

