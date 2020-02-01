// set RX & TX the same for picoUART 1-wire simplex mode
#define PU_TX B,3
#define PU_RX B,3

//#define BAUD_RATE 9600
#include "picoUART.h"
#include <util/delay.h>
#include <avr/pgmspace.h>


void prints_P(const __flash char* s)
{
    char c;
    //while (c = pgm_read_byte(s++))
    while (c = *s++)
        pu_tx(c);
}

void main()
{
    while (1) {
        prints_P(PSTR(" picoUart\r\n"));
        char c = pu_rx();
        //pu_rxtx_wait();
        pu_tx(c);
    }
}

