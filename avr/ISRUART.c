// (c) Ralph Doncaster 2020
// Non-commercial use and modification of this code is permitted.
// Contact ralphdoncaster at gmail for commercial use requests.
// Any re-distribution must include this notice.
//
// AVR WGM full-duplex UART using timer/counter0
// Maximum baud rate is 1/100th of clock rate, i.e. 80kbps for 8Mhz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// global for write
__attribute(( section(".noinit") )) uint8_t wgm_txdata;
__attribute(( section(".noinit") )) uint8_t wgm_rxdata;

#ifndef OC0A_BIT
#define OC0A_BIT PB0
#define OC0A_DDR DDRB
#endif

#define BAUD_RATE 38400

#define WGMRXBIT 1

const int LED = 4;

#define DIVIDE_ROUNDED(NUMBER, DIVISOR) ((((2*(NUMBER))/(DIVISOR))+1)/2)
#define PRESCALER 8L
#define TICKS_PER_BIT DIVIDE_ROUNDED(F_CPU, PRESCALER * BAUD_RATE)

#if (TICKS_PER_BIT > 171)
#error badurate too low
#endif
#if (TICKS_PER_BIT * PRESCALER < 100)
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

// Tx ISR 44 cycles incl reti
ISR(TIM0_COMPA_vect, ISR_NOBLOCK)
//ISR(TIMER0_COMPA_vect)
{
    uint8_t ch = wgm_txdata;
    if (ch == 0) {                      // tx finished
        //TCCR0A = 0;                   // disconnect OC0A
        TIMSK &= ~(1<<OCIE0A);
        return;
    }

    if (ch & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    ch >>= 1;
    wgm_txdata = ch;
    OCR0A += TICKS_PER_BIT;
    debug();
}

// Rx start ISR
ISR(PCINT0_vect)
{
    debug();
    // 45 cycle ISR overhead calculated from disassembly
    uint8_t isr_overhead_ticks = DIVIDE_ROUNDED(45, PRESCALER);
    uint8_t first_bit_ticks = (TICKS_PER_BIT * 1.5) - isr_overhead_ticks;
    OCR0B = TCNT0 + first_bit_ticks;
    PCMSK &= ~(1<<WGMRXBIT);            // turn off PCINT
    wgm_rxdata = 0x80;                  // setup bit shift counter
    TIFR = 1<<OCF0B;                    // clear OC0B flag
    TIMSK |= 1<<OCIE0B;                 // enable Rx timer ISR
}

// Rx bit ISR 42c incl reti
ISR(TIM0_COMPB_vect)
{
    debug();
    uint8_t data = wgm_rxdata;
    uint8_t lastbit = data & 0x01;
    data /= 2;                          // shift right
    if (PINB & (1<<WGMRXBIT) )
        data |= 0x80;
    wgm_rxdata = data;
    if (lastbit)
        TIMSK &= ~(1<<OCIE0B);          // disable ISR
    OCR0B += TICKS_PER_BIT;             // set time for next bit
}

// returns true when there is data to read
uint8_t rx_data_ready()
{
    // data is ready if PCINT & TIM0_COMPB disabled
    return !(PCMSK & 1<<WGMRXBIT) && !(TIMSK & 1<<OCIE0B);
}

uint8_t rx_read()
{
    uint8_t data = wgm_rxdata;
    // wait for stop bit/idle
    loop_until_bit_is_set(PINB, WGMRXBIT);
    PCMSK |= 1<<WGMRXBIT;               // enable Rx ISR
    return data;
}

void UARTsetup() {
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

