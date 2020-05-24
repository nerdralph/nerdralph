// t13 timer ISR transmit using OC0A 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// global for write
__attribute(( section(".noinit") ))
uint8_t outChar;

#define OC0A PB0

#define BAUD_RATE 38400

#define DIVIDE_ROUNDED(NUMBER, DIVISOR) ((((2*(NUMBER))/(DIVISOR))+1)/2)
// #define BITTIME DIVIDE_ROUNDED(F_CPU, BAUD_RATE)
#define PRESCALER 8
#define TICKS_PER_BIT DIVIDE_ROUNDED(F_CPU, PRESCALER * BAUD_RATE)
#if (TICKS_PER_BIT > 256)
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

#ifdef TIMSK0
#define TIMSK TIMSK0
#define TIFR TIFR0
#endif

ISR(TIM0_COMPA_vect)
//ISR(TIMER0_COMPA_vect)
{
    debug();
    //TIFR = 1<<OCF0A;              // clear flag
    uint8_t ch = outChar;
    if (ch == 0) {                      // tx finished
        TIMSK = 0;
        //TCCR0A = 0;                     // disconnect OC0A
        //TIMSK0 &= ~(1<<OCIE0A);
        return;
    }

    if (ch & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    ch >>= 1;
    outChar = ch;
    //OCR0A += BITTIME;
    debug();
}

void UARTsetup() {
    //PORTB |= 1<<OC0A;                   // idle state = high
    TCCR0A = SET_ON_MATCH;
    OCR0A = TICKS_PER_BIT;
    // start timer0 /8 prescaler, force compare
    TCCR0B = 1<<CS01 | 1<<FOC0A;
    DDRB |= 1<<OC0A;
}

//__attribute(( noinline ))
void write(uint8_t c) {
    while (TIMSK & 1<<OCIE0A);         // wait for last tx to finish
    // OC0A still running in SET_ON_MATCH mode during idle
    // stop bit time is finished if OCF0A set
    while ((TIFR & 1<<OCF0A) == 0);
    // todo: disable/enable interrupts
    TIFR = 1<<OCF0A;
    TCCR0A = CLEAR_ON_MATCH;            // setup for start bit
    TCNT0 = 0;                          // or TCNT0 = 1 after FOC?
    // timer0 /8 prescaler, force compare
    TCCR0B = 1<<CS01 | 1<<FOC0A;

    // if first bit is set, switch to SET_ON_MATCH
    if (c & 0x01)
        TCCR0A = SET_ON_MATCH;

    // high bit flag for end of byte
    outChar = (c>>1) | 0x80;
    TIMSK = 1<<OCIE0A;
}

void main() {
    UARTsetup();
    //DDRB |= 1<<PB2;             // debug
    sei();
    _delay_ms(1000);
    const unsigned ovf_per_sec = F_CPU / PRESCALER / 256;
    uint32_t overflows = 0;
    while (1) {
        /*
        if (TIFR & 1<<TOV0) {
            TIFR = 1<<TOV0;       // clear overflow flag
            overflows++;
        }
        */
        //if ( overflows == (ovf_per_sec / 4) ) {
        TIFR = overflows & 0x1;
        if ( ++overflows == 1L<<18) {
            overflows = 0;
            write('@');
            write(0x55);
            write(0x20);
        }
        //_delay_us(1);
    }
}

