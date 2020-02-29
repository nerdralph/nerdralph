// adc picoTouch
 
#include <avr/io.h>
#include <util/delay.h>
 
int LED = 4; 
int TOUCH = 3;

uint8_t adc_touch()
{
    uint8_t samples = 4;
    uint8_t level = 0;

    do {
        // ADC3/PB3
        ADMUX = (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);

        DDRB |= (1 << TOUCH);               // discharge
        // enable ADC & discharge S/H cap
        ADCSRA |= (1 << ADEN);
        _delay_us(1);                       // S/H RC time
        ADCSRA = 0;                         // ADC off
        PORTB |= (1 << TOUCH);              // charge touchpad

        DDRB &= ~(1 << TOUCH);              // input mode
        PORTB &= ~(1 << TOUCH);             // pullup off

        // enable ADC with /16 prescaler, equalize S/H cap charge
        ADCSRA = (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0)
            | (1 << ADEN);
        _delay_us(1);                       // S/H RC time

        ADCSRA |= (1 << ADSC);
        while ( bit_is_set(ADCSRA, ADSC) );
        level += ADCH;
        ADCSRA = 0;                         // ADC off
    } while (--samples);
    return level;
}

int main (void)
{
    DDRB |= (1<<LED);

    uint8_t base = adc_touch();
    while (1) {
        uint8_t level = adc_touch();
        if (level > (uint8_t)(base + 32)) {
            PINB |= (1<<LED);
            _delay_ms(500);
        }
    }
}
