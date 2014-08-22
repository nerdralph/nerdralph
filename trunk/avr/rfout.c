/* rf output using PLL */
#define F_CPU 9860000L

#include <avr/io.h>
#include <util/delay.h>

__attribute__ ((OS_main)) int main()
{
  // OSCCAL -= 6; // gives ~8Mhz @ 5V
  OSCCAL += 23; // gives 9.86Mhz
  PLLCSR = (1<<PCKE) | (1<<PLLE); // turn on PLL
  // setup timer1
//  OCR1A = 0; // match count - reset value = 0
//  OCR1C = 16; // count to
// OCR1C reset value = 255
  DDRB = (1<<PB1);  // set OC1A to output
  while(1){
    // 150 + 130 us =~ 3.5kHz cycle tone generator
    TCCR1 = (1<<CTC1)|(1<<COM1A0)|(1<<CS10);
    _delay_us(150);
//    TCCR1 = 0;
//    PORTB = 0;
    _delay_us(130);
  }
}
