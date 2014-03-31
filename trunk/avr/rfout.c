/* rf output using PLL */
#define F_CPU 9860000L

#include <avr/io.h>
#include <util/delay.h>

void main()
{
  // OSCCAL -= 6; // gives ~8Mhz @ 5V
  OSCCAL += 23; // gives 9.86Mhz
  PLLCSR = (1<<PCKE) | (1<<PLLE); // turn on PLL
  // setup timer1
//  OCR1A = 0; // match count 
  OCR1C = 0; // count to
  DDRB = (1<<PB1);  // set OC1A to output
  while(1){
    TCCR1 = (1<<CTC1)|(1<<COM1A0)|(1<<CS10);
    _delay_ms(30);
    TCCR1 = 0;
    PORTB = 0;
    _delay_ms(50);
  }
}
