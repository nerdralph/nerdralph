#define DTXWAIT 3

#include "dbgtx.h"
#include <avr/pgmspace.h>

int main()
{
    dprint( PSTR("Hello debug\n") );
    dprinthex(PORTB);
    dprint( PSTR("Bye debug\n") );
}
