/* tiny touch sense example for AVR MCUs
 * touch sense code takes only 40 bytes of flash
 * <100 bytes flash  use on a t13 with interrupt vectors and main overhead
 * Ralph Doncaster 2020 open source MIT license
 */

#include <avr/io.h>
#include <util/delay.h>

const uint8_t TOUCH_GPIO = 3;
const uint8_t LED_GPIO = 4;

// valid rise time range = 1-255
// zero indicates overflow; likely excessive capacitance or a short
// GPIO = 8-bit port mask i.e. (1 << PB4)
uint8_t rise_time(uint8_t gpio)
{
    uint8_t time = 0;
    // we need more samples at lower clock speeds
    uint8_t samples = (uint8_t)(24e6/F_CPU + 0.5);
    do {
        DDRB |= gpio;               // discharge pad
        DDRB &= ~(gpio);            // input mode
        do {
            PINB = gpio;            // pullup on
            PINB = gpio;            // pullup off
            time++;
        } while ( time && !(PINB & gpio) );
    } while (--samples);
    return time;
}

int main()
{
    DDRB |= (1<<LED_GPIO);              // output mode
    uint8_t threshold = 22;
    while (1) {
        uint8_t delay = 0;
        if ( rise_time(1 << TOUCH_GPIO) > threshold ) {
            PINB |= (1<<LED_GPIO);      // toggle LED
            _delay_ms(1000);            // wait for touch release
        }
        /*
        uint8_t t2 = rtime(1 << PB2);
        if ( t2 > 28 ) {
            // eelog(t2);
            PINB |= (1<<LED_GPIO);      // toggle LED
            wait_ms(1000);              // wait for touch release
        }
        */
    }
}

