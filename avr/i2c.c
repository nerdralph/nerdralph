/* Small i2c master for AVR devices
 * Ralph Doncaster (c) 2019
 * free software - MIT license
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

int i2c_write(uint8_t data);

#define I2C_PORT PORTB 
#define I2C_DDR DDRB
#define I2C_PIN PINB

// PORT pin numbers for SDA & SCL (0-7)
const int I2C_SCL = 3;  //physical pin 2
const int I2C_SDA = 4;  //physical pin 3

void i2c_start(){
    // set I2C_SDA to output mode for low 
    I2C_DDR |= (1<<I2C_SDA);
    // set I2C_SCL to output mode for low 
    I2C_DDR |= (1<<I2C_SCL);
}

int i2c_start_write(uint8_t addr){
    i2c_start();
    return i2c_write(addr<<1);      // address goes in upper 7 bits
}

int i2c_start_read(uint8_t addr){
    i2c_start();
    return i2c_write(addr<<1 | 1);  // address goes in upper 7 bits
}

void i2c_stop(){
    // set I2C_SCL to input mode for high 
    I2C_DDR &= ~(1<<I2C_SCL);
    // set I2C_SDA to input mode for high 
    I2C_DDR &= ~(1<<I2C_SDA);
}

inline void scl_hi(){
    // set I2C_SCL to input mode for high
    I2C_DDR &= ~(1<<I2C_SCL);
    // check for clock stretching
    while ((I2C_PIN & (1<<I2C_SCL)) == 0);
}

// start must be called first
int i2c_write(uint8_t data){
    uint8_t i = 8;
    const uint8_t i2c_low = I2C_DDR | (( (1<<I2C_SCL) | (1<<I2C_SDA) ));
    do{
        // set I2C_SDA to input mode for high
        if (data & 0x80) I2C_DDR &= ~(1<<I2C_SDA);
        // scl_hi();
        // set I2C_SCL to input mode for high
        I2C_DDR &= ~(1<<I2C_SCL);
        data <<= 1;
        // check for clock stretching
        while ((I2C_PIN & (1<<I2C_SCL)) == 0);
        /*
        // set I2C_SCL to output mode for low 
        I2C_DDR |= (1<<I2C_SCL);
        // set I2C_SDA to output mode for low 
        I2C_DDR |= (1<<I2C_SDA);
        */
        I2C_DDR = i2c_low;

    } while (--i);

    // release I2C_SDA
    I2C_DDR &= ~(1<<I2C_SDA);
    scl_hi();
    // check for ACK
    int err = 0;
    // NACK = SDA high
    if (I2C_PIN & (1<<I2C_SDA)) err = -1;
    // set I2C_SCL to output mode for low 
    I2C_DDR |= (1<<I2C_SCL);
    // set I2C_SDA to output mode for low 
    I2C_DDR |= (1<<I2C_SDA);
    return err;
}

// weak main for testing
__attribute__ ((weak))\
void main() {
    // scan bus
    uint8_t addr = 8;
    const uint8_t addr_end = 119;
    do {
        if (i2c_start_write(addr++) == 0) {
            // ack received, turn on LED
            PORTB |= (1<<0);    // pullup PB0
        }
        i2c_stop();
        _delay_us(50);          // leave bus idle
    } while (addr <= addr_end);
    if (PORTB == 0) while (1) {
        // no devices found, flash LED
        PINB |= (1<<0);    // pullup PB0
        _delay_ms(200);
    }
}

