#define BAUD_RATE 38400

#include "picoUART.h"
#include <util/delay.h>

void puts(const char* s)
{
    while (*s) pu_tx(*s++);
}

void wait2s() { _delay_ms(2000); }

void main()
{
    while (1) {
        puts("picoUart\r\n");
        wait2s();
    }
}
