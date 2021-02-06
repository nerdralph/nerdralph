#include <util/delay.h>
#include "bbserial.h"

void main()
{
    CyclesPerBit = BIT_CYCLES(57600L);
    while (1) {
        char c = '0';
        do {
            WriteB(c++);
        } while (c <= 'Z');
        WriteB('\n');
        _delay_ms(1000);
    }
}

