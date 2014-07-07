/* Ralph Doncaster 2014 public domain software
 * 1s timer
 */

#define byte unsigned char
#define F_CPU 16000000

#include <avr/io.h>
#include <time.h>

// seconds defined in assembler code
extern unsigned long seconds;
//extern byte ovfl_count;

// to-do: map prescaler /32 thru /256 to 2^CSn

#define CSDIV256 (1<<CS22) | (1<<CS21)

void main()
{
    struct tm * timeptr;

    time_t timer = time(NULL);
    timeptr = localtime(&timer);
    GPIOR0 = timeptr->tm_hour;

    TCCR0A = (1<<COM0A1) | (1<<COM0A0) | (1<<WGM01) | (1<<WGM00);
    TCCR2B = CSDIV256;
    OCR0A = 64;                 // set OC0A at count 64
}
