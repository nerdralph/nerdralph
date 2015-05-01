/* AVR half-duplex software SPI master
 * @author: Ralph Doncaster 2015 public domain software
 * connect 330 Ohm resistor between slave MO and MI pins and
 * connect slave MOSI to AVR MO/MI
 *  AVR              SLAVE
 *  SCK ------------ SCK
 *  MOMI --+-------- MOSI
 *         +-\/\/\-- MISO
 *            330
 */


#define cbi(x,y)    x&= ~(1<<y)
#define sbi(x,y)    x|= (1<<y)

#ifndef SPI_PORT
#define SPI_MOMI (SPI_SCK - 1)    
#endif

#define SPI_DDR (*((&SPI_PORT) -1))
#define SPI_PIN (*((&SPI_PORT) -2))

uint8_t spi_byte(uint8_t byte){

    uint8_t i = 8;
    sbi (SPI_DDR, SPI_SCK);        // output mode
    
    do{
        sbi (SPI_DDR, SPI_MOMI);        // output mode
        cbi (SPI_PORT, SPI_MOMI);
        if(byte & 0x80) sbi (SPI_PORT, SPI_MOMI);
        sbi (SPI_PORT, SPI_SCK);
        cbi (SPI_DDR, SPI_MOMI);        // input mode
        byte <<= 1;                     
        asm("nop");
        if(SPI_PIN & (1<<SPI_MOMI)) byte++;
        cbi(SPI_PORT, SPI_SCK);
    
    }while(--i);

    return byte;
}

