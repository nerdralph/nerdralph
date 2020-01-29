// for picoUART 1-wire half duplex
#define PU_TX B,1
#define PU_RX B,1

#include "picoUART.h"
#include <util/delay.h>

void print(const char* s)
{
    while (*s) pu_tx(*s++);
}

void wait1s() { _delay_ms(1000); }

void main()
{
    while (1) {
        print(" picoUart\r\n");
        char c = pu_rx();
        _delay_us(100);
        pu_tx(c);
    }
}

