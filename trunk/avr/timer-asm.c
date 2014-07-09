/* Ralph Doncaster 2014 public domain software
 * 1s timer
 */

#define byte unsigned char

#include <avr/io.h>

// seconds defined in assembler code
extern unsigned long __system_time;

// to-do: map prescaler /32 thru /256 to 2^CSn
#define CSDIV256 (1<<CS22) | (1<<CS21)

void main()
{

    // normal mode, clear counter when count reached 

    TCCR2A = (1<<WGM21);
    TCCR2B = CSDIV256;

    OCR2A = 250;                // reset timer when count reached
    TIMSK2 = (1<<OCIE2A);       // enable intterupt

    DDRB = 0xff;                // output mode
    while (1) {
        PORTB = __system_time & 0xff;
    }
}
