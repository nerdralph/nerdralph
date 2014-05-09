/* 3/6mbps 8N1 serial UART using synchronous mode - requires 12Mhz clock
 * @author: Ralph Doncaster 2014
 */

#include <avr/io.h>
#include <util/delay.h>

#define XCKPIN 4
#define XCKDDR DDRD

static void uartInit()
{
    XCKDDR |= (1<<XCKPIN);       // output mode for master 
    UCSRC = (1<<URSEL)|(1<<UMSEL)|(1<<UCSZ1)|(1<<UCSZ0);
    UCSRB |= (1<<TXEN);
#ifdef HALF_SPEED
    UBRRL = 1;
#endif
}

void main()
{
    char c;
    uartInit();

    while(1) {
        _delay_us(120);   // idle >1 frame for synchonization
        for ( c = 0x7f; c >= 0x20; --c) {
            while ( !(UCSRA & (1<<UDRE)) );
            UDR = c;
        }
    }
}
