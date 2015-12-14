/* AVR software SPI master
 * @author: Ralph Doncaster 2015 public domain software
 * define SPI_PORT and SPI_SCK before including this file
 */

#include <avr/io.h>

#ifndef SPI_SCK
#define SPI_SCK 2
#endif
#ifndef SPI_MISO
#define SPI_MISO (SPI_SCK - 1)    
#endif
#ifndef SPI_MOSI
#define SPI_MOSI (SPI_MISO - 1)    
#endif

#ifndef SPI_PORT
#define SPI_PORT PORTB
#endif
#define SPI_DDR (*((&SPI_PORT) -1))
#define SPI_PIN (*((&SPI_PORT) -2))

void spi_setup()
{
    SPI_DDR |= ((1<<SPI_SCK) | (1<<SPI_MOSI));
}

void spi_byte(uint8_t byte)
{
    uint8_t i = 8;
    uint8_t portstate = SPIPORT;

    do{
        if (byte & 0x80) SPI_PIN = (1<<SPI_MOSI);
        SPI_PIN = (1<<SPI_SCK);         // clk hi
        byte <<= 1;
        // read miso - comment out line for output only
        if (SPI_PIN & (1<<SPI_MISO)) byte++;
    	SPIPORT = portstate;      // clk and data low
    }while(--i);
}

