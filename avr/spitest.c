#include <avr/io.h>
#include <util/delay.h>
#include "u8tohex.h"
#include "bbuart/BBUart.h"

#define SPI_PORT PORTB
#define SPI_SCK 2
#define SPI_MOMI 0

#include "halfduplexspi.h"

void print2chars(uint16_t chars)
{
    TxByte(chars>>8);
    TxByte(chars & 0xff);
}

void main()
{
    uint8_t result, address;
    spi_setup();
loop:
    address = 0x0f;

    do {
        cbi (SPI_PORT, SPI_SCK);      // discharge SCK->CSN RC
        print2chars(u8tohex(address));
        TxByte(' ');

        spi_byte(address);
        result = spi_byte(address);

        sbi (SPI_PORT, SPI_SCK);      // charge SCK->CSN RC

        print2chars(u8tohex(result));

        TxByte('\r');
        TxByte('\n');
    } while ( address-- );

    _delay_ms(1000);
    goto loop;
}
