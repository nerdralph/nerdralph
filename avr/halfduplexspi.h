/* AVR half-duplex software SPI master
 * @author: Ralph Doncaster 2015 public domain software
 * connect 330 Ohm resistor between slave MO and MI pins and
 * connect slave MOSI to AVR MO/MI
 *  AVR              SLAVE
 *  SCK ------------ SCK
 *  MOMI --+-------- MOSI
 *         +-\/\/\-- MISO
 *            330
 *
 * define TDDSPI before including this file to enable time-division
 * duplexing.  Otherwise set SPI_MOMI to input or output mode before
 * calling spi_byte.
 */


#define cbi(x,y)    x&= ~(1<<y)
#define sbi(x,y)    x|= (1<<y)

#ifndef SPI_PORT
#define SPI_MOMI (SPI_SCK - 1)    
#endif

#define SPI_DDR (*((&SPI_PORT) -1))
#define SPI_PIN (*((&SPI_PORT) -2))

void spi_setup()
{
    sbi (SPI_DDR, SPI_SCK);             // output mode
    //sbi (MCUCR, PUD);                   // disable pullup
}

uint8_t spi_byte(uint8_t dataout){

    uint8_t datain, bits = 8;
    
    do{
        datain <<= 1;                     
        if(SPI_PIN & (1<<SPI_MOMI)) datain++;

        sbi (SPI_DDR, SPI_MOMI);        // output mode
        if (dataout & 0x80) sbi (SPI_PORT, SPI_MOMI);
        SPI_PIN = (1<<SPI_SCK);
        cbi (SPI_DDR, SPI_MOMI);        // input mode
        SPI_PIN = (1<<SPI_SCK);         // toggle SCK

        cbi (SPI_PORT, SPI_MOMI);
        dataout <<= 1;                     
    
    }while(--bits);

    return datain;
}

uint8_t spi_in(void){

    uint8_t pinstate, datain, bits = 8;
    
    do{
        datain <<= 1;                     
        SPI_PIN = (1<<SPI_SCK);
        pinstate = SPI_PIN;
        SPI_PIN = (1<<SPI_SCK);         // toggle SCK
        if(pinstate & (1<<SPI_MOMI)) datain++;
    }while(--bits);

    return datain;
}

void spi_out(uint8_t dataout){

    sbi (SPI_DDR, SPI_MOMI);        // output mode
    uint8_t bits = 8;
    
    do{
        if (dataout & 0x80) SPI_PIN = (1<<SPI_MOMI);
        SPI_PIN = (1<<SPI_SCK);
        SPI_PIN = (1<<SPI_SCK);         // toggle SCK
        cbi (SPI_PORT, SPI_MOMI);
        dataout <<= 1;                     
    }while(--bits);

    cbi (SPI_DDR, SPI_MOMI);        // input mode
}

