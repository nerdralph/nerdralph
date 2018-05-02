#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define byte unsigned char

/*
ISR(WDT_vect){
    PINB = 1<<PB1;       // toggle PB1
}
*/

// EMPTY_INTERRUPT(WDT_vect)

#define TPS 64
// 64 ticks per second
void sleep(byte ticks){
    do{
        char wdtcr;
        do {
            wdtcr = WDTCR;
        } while (!(wdtcr & 1<<WDTIF));
        WDTCR = wdtcr;              // reset WDTIF
    }
    while (ticks--);
}

// same bit but different define on t13 vs other tinies 
#define WDISREN 6

//void main(void) __attribute__ ((OS_main));

void main(){
    // clear reset flags 
    // MCUSR = 0;

    // enable WD 
    //WDTCR = (1<<WDCE) | (1<<WDE);
    //WDTCR = (1<<WDE) | (1<<WDP2) | (1<<WDP1);
#ifdef WDTIE
    WDTCR = 1<<WDISREN;
#endif
    //DDRB = 1 << PB5 | 1 << PB0;
    DDRB = 1<<PB1 | 1<<PB0;
    volatile char c=0;
    //sei();
    //MCUCR = 1<<SE;
    while (1){
        sleep(TPS/4);
        //_delay_ms(250);
        //sleep_cpu();
        PINB = 1;       // toggle PB0
/*
        PORTB |= 1<<PB0;
        _delay_ms(250);
        //sleep(TPS/4);
        PORTB &= ~1<<PB0;
*/
        c++;
    }
}
