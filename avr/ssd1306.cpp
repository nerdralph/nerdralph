/* ssd1306 test for picoI2C
 * Ralph Doncaster (c) 2019
 * free software - MIT license
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "picoI2C-09.h"


// PORT pin numbers for SDA & SCL (0-7)
const uint8_t PicoI2C::SCL = 4;
const uint8_t PicoI2C::SDA = 3;

const uint8_t ssd1306_init [] PROGMEM = {
    0xC8,               // COM output scan direction
    0xA1,               // segment re-map A1=column 127 mapped to SEG0. 
    0xA8, 0x1F,         // multiplex ratio
    0xDA, 0x02,         // com pins config
    0x8D, 0x14,         // enable DC charge pump
    //0xA7,               // invert display
    0xAF                // display ON
};


int main()
{
    // scan bus
    uint8_t addr = 0x3C;
    if (PicoI2C::startWrite(addr) == 0) {
        // turn on LED to show ACK received
        PORTB |= (1<<0);                // pullup PB0
        
        PicoI2C::write(0x00);           // command sequence
        uint8_t init_len = sizeof(ssd1306_init);
        uint8_t pos = 0;
        while (pos < init_len) {
            uint8_t data = pgm_read_byte(&ssd1306_init[pos++]);
            PicoI2C::write(data);
        }
        /*
        uint8_t data;
        while ( data = pgm_read_byte(&ssd1306_init[pos++]) )
            PicoI2C::write(data);
        PicoI2C::dprint(&ssd1306_init);
        */

        PicoI2C::stop();

        PicoI2C::startWrite(addr);
        PicoI2C::write(0x40);           // data sequence
        uint8_t count = 0;
        while (++count) {
            PicoI2C::write(count);
            _delay_ms(25);
        }
    }
    PicoI2C::stop();
    while (1);                          // done
}
