#include <avr/io.h>
#include <avr/interrupt.h>

// global for putc
register char outChar asm("r15");

#define OC0A PB0

#define DIVIDE_ROUNDED(NUMBER, DIVISOR) ((((2*(NUMBER))/(DIVISOR))+1)/2)
#define BITTIME DIVIDE_ROUNDED(F_CPU, 115200)
#if (BITTIME > 255)
#error badurate too low
#endif

#define CLEAR_ON_MATCH (1<<COM0A1) 
#define SET_ON_MATCH ((1<<COM0A1) | (1<<COM0A0))

ISR(TIM0_COMPA_vect){
    char c = outChar;
    if (c == 0){            // tx finished
        TIMSK0 = 0;
        return;
    }
    uint8_t tccrv = CLEAR_ON_MATCH;
    if (c & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    outChar = c>>1;
    OCR0A += BITTIME;
}

void putc(char c){
    PORTB &= ~(1<<OC0A);        // start bit
    OCR0A = TCNT0 + BITTIME;
    // setup output for first bit
    if (c & 0x01)
        TCCR0A = SET_ON_MATCH;
    else
        TCCR0A = CLEAR_ON_MATCH;

    // high bit flag for end of byte
    outChar = (c>>1) | 0x80;
    TIMSK0 = 1<<OCIE0A;
}

void main(){
    sei();
    char c = PINB;
    putc(c);
}
