/* usb sniffer - captures 2 or 8 bits on PORTB
 * 2-bit mode selected if Rx pin (PD0) held low
 * 6mbps 8N1 serial UART using synchronous mode - requires 12Mhz clock
 * @author: Ralph Doncaster 2014
 */

#include <avr/io.h>
#include <util/delay.h>

#define XCKPIN 4
#define XCKDDR DDRD

extern void capture8();
extern void capture2();

static void uartInit()
{
    XCKDDR |= (1<<XCKPIN);       // output mode for master 
    UCSRC = (1<<URSEL)|(1<<UMSEL)|(1<<UCSZ1)|(1<<UCSZ0);
    UCSRB |= (1<<TXEN);
}

static void waitTrigger()
{
    char sample1, sample2;
    sample1 = PINB;
    do {
        sample2 = PINB;
    } while (sample1 == sample2);
}

void main()
{
    uartInit();
    _delay_us(3);       // need idle > 1 frame to ensure sync
    waitTrigger();
    if (PORTD & 0x01){
        capture8();          // call main asm routine
    } else {
        capture2();          // call main asm routine
    }
}
