#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// global for putc
//uint8_t outChar;
register uint8_t outChar asm("r15");

#define OC0A PB0

#define DIVIDE_ROUNDED(NUMBER, DIVISOR) ((((2*(NUMBER))/(DIVISOR))+1)/2)
#define BITTIME DIVIDE_ROUNDED(F_CPU, 115200)
#if (BITTIME > 255)
#error badurate too low
#endif

// WGM01 = CTC mode
#define CLEAR_ON_MATCH ((1<<COM0A1) | (1<<WGM01))
#define SET_ON_MATCH ((1<<COM0A1) | (1<<COM0A0) | (1<<WGM01))

#ifdef DEBUG
#define debug() PINB = 1<<PB2
#else
#define debug() {}
#endif

ISR(TIM0_COMPA_vect) {
    debug();
    if (outChar == 0) {               // tx finished
        TIMSK0 &= ~(1<<OCIE0A);
        return;
    }

    if (outChar & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    outChar >>= 1;
    //OCR0A += BITTIME;
    debug();
}

void UARTsetup() {
    TCCR0A = SET_ON_MATCH;
    OCR0A = BITTIME;
    // start timer0 no prescaler, force compare
    TCCR0B = 1<<CS00 | 1<<FOC0A;
    DDRB |= 1<<OC0A;
}

void putc(uint8_t c) {
    while (TIMSK0 & 1<<OCIE0A);
    while ((TIFR0 & 1<<OCF0A) == 0);    // wait for last tx to finish
    TIFR0 = 1<<OCF0A;
    TCNT0 = 0;
    TCCR0A = CLEAR_ON_MATCH;            // setup for start bit
    // timer0 no prescaler, force compare
    TCCR0B = 1<<CS00 | 1<<FOC0A;

    // setup output for first bit
    if (c & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    // high bit flag for end of byte
    outChar = (c>>1) | 0x80;
    TIMSK0 |= 1<<OCIE0A;
}

void main() {
    UARTsetup();
    DDRB |= 1<<PB2;             // debug
    sei();
    while (1) {
        putc(0x55);
        putc(0xAA);
        _delay_ms(10);
    }
}
