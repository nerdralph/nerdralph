#include <avr/io.h>
#include <util/delay.h>

void main(){
    DDRB = 0xff;
    while (1) {
        PINB = 0xff;
        _delay_ms(250);
    }
}
