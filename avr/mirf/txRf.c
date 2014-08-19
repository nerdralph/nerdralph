#include "mirf.h"
#include "nRF24L01.h"
#include <util/delay.h>

/* packet buffer */
//static uint8_t buf[mirf_PAYLOAD] = {0xde, 0xad, 0};
static uint8_t buf[mirf_PAYLOAD];


const uint8_t mirf_config_items[] = {
    EN_AA, 0,
    LAST_ITEM, 0
};

void main(void)
{
    mirf_init();

    // wait for mirf - is this necessary?
//    _delay_ms(50);

    mirf_config();

//    mirf_config_register(SETUP_RETR, 0 ); // no retransmit 
    // disable enhanced shockburst
    mirf_config_register(EN_AA, 0 ); // no auto-ack 
//    mirf_config_register(RF_SETUP, (1<<RF_DR_LOW) ); // low spd & power 

    while (1){
        /* tie LED to CSN for diag */
        mirf_CSN_lo;
        _delay_ms(100);
        mirf_CSN_hi;

        // need time to come out of power down mode
        // s. 6.1.7, table 16
        // datasheet says 1.5ms max, tested as low as 600us
        mirf_config_register(CONFIG, mirf_CONFIG | (1<<PWR_UP) );
        _delay_ms(1);

        mirf_send(buf, mirf_PAYLOAD);
        // clear PWR_UP after CE high time
        // in practice, seems to work without it
        _delay_us(10);
        mirf_config_register(CONFIG, mirf_CONFIG );
        _delay_ms(1000);

        // Reset status register for further interaction
//        mirf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT)); // Reset status register
        mirf_config_register(STATUS,(1<<TX_DS)); // Reset status register
    }
}
