// set RX & TX the same for picoUART 1-wire simplex mode
#define PU_TX B,3
#define PU_RX B,4

#define PU_BAUD_RATE 500000
#include "picoUART.h"
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>


//uint8_t purx_data;                      // RX ISR storage

//uint8_t somedata;

/*
void prints_P(const __flash char* s)
{
    char c;
    //while (c = pgm_read_byte(s++))
    while (c = *s++)
        pu_tx(c);
}
*/

void rx_enable()
{
    PCMSK = 1<<bit(PU_RX);
    GIMSK = 1<<PCIE;
}

// put rx_read in .S
uint8_t rx_read()
{
    uint8_t data = purx_data;
    loop_until_bit_is_set(pin(PU_RX), bit(PU_RX));
    rx_enable();
    return data;
}

int rx_data_ready()
{
    // data is ready if PCINT disabled
    return !(PCMSK & 1<<bit(PU_RX));
}

// disable rx during tx
void txnorx(uint8_t c)
{
    PCMSK = 0;
    pu_tx(c);
    rx_enable();
}

void print_binary(uint8_t val)
{
    uint8_t bits = 8;
    do {
        if (val & 0x80) { txnorx('1');}
        else { txnorx('0');}
        val <<= 1;
    } while(--bits);
    txnorx('\n');
}

void main()
{
    // works, but compiler doesn't know somedata is used
    //asm("sts (somedata), r1");
    //asm ("sts %0, r1" :: "M"(_SFR_MEM_ADDR(SREG)));
    //asm ("sts %0, r1" : "=m"(somedata));
    //OSCCAL+=2;      // ~9.6M
    prints_P(PSTR(" picoUart\r\n"));
    //rx_enable();
    //asm("rjmp (_pu_tx)");
    while (1) {
        //somedata++;
        char c = pu_rx();
        //pu_rxtx_wait();
        pu_tx(c);
        //pu_tx(somedata);
        pu_tx('\n');
        /*
        if ( rx_data_ready() ) {
            //pu_rxtx_wait();
            //txnorx(rx_read());
            pu_tx(rx_read());
            //pu_tx('\n');
            //print_binary(rx_read());
        }
        */
    }
}

