#include "mirf.h"
#include <util/delay.h>

/* packet buffer */
uint8_t buf[mirf_PAYLOAD];

void main(void)
{
    uint8_t pktReceived = 0;

    mirf_init();

    // wait for mirf - is this necessary?
    _delay_ms(50);

    mirf_config();

    while (1){
        pktReceived = mirf_data_ready();
        mirf_get_data(buf);
        mirf_CSN_lo;
        _delay_ms(50);
        /* tie LED to CSN for diag */
        if (pktReceived) _delay_ms(1500);
        mirf_CSN_hi;
        _delay_ms(1000);
    }
}
