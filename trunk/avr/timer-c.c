/* Ralph Doncaster 2014 public domain software
 * 1s timer
 */

#define byte unsigned char
#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned long seconds;

/* ISR setup to run at count 250
 * x prescaler 256 = every 64,000 cycles
 */

ISR(TIMER2_COMPA_vect)
{
    static byte ovfl_count;
    if ( ++ovfl_count == ((F_CPU)/64000) )
    {
        ovfl_count = 0;
        seconds++;
    }
}

// to-do: map prescaler /32 thru /256 to 2^CSn

#define CSDIV256 (1<<CS22) | (1<<CS21)

void main()
{
    TCCR0A = (1<<COM0A1) | (1<<COM0A0) | (1<<WGM01) | (1<<WGM00);
    TCCR2B = CSDIV256;
    OCR0A = 64;                 // set OC0A at count 64
}
