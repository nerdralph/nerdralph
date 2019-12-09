/* Small i2c master for AVR devices
 * Ralph Doncaster (c) 2019
 * free software - MIT license
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define I2C_PORT PORTB 
#define I2C_DDR DDRB
#define I2C_PIN PINB

const int SUCCESS = 0;
const int ADDR_NAK = 2;
const int BUS_INUSE = 5;

// PORT pin numbers for SDA & SCL (0-7)
const int I2C_SCL = 3;  //physical pin 2
const int I2C_SDA = 4;  //physical pin 3

static uint8_t i2c_start()
{
    // SDA should be high if bus is free
    if ((I2C_PIN & (1<<I2C_SDA)) == 0)
        return BUS_INUSE;
    // set I2C_SDA to output mode for low 
    I2C_DDR |= (1<<I2C_SDA);
    asm ("lpm" ::: "r0");       // 3 cycle delay
    // set I2C_SCL to output mode for low 
    I2C_DDR |= (1<<I2C_SCL);
    return 0;
}

__attribute__((always_inline))\
static inline void scl_hi()
{
    // set I2C_SCL to input mode for high
    I2C_DDR &= ~(1<<I2C_SCL);
    // check for clock stretching
    while ((I2C_PIN & (1<<I2C_SCL)) == 0);
}

// combined i2c read/write routine
// pass 0xFF when reading
static uint8_t i2c_rw(uint8_t data)
{
    uint8_t bits = 8;
    do{
        I2C_DDR &= ~(1<<I2C_SDA);   // SDA high
        if (!(data & 0x80)) I2C_DDR |= (1<<I2C_SDA);
        scl_hi();
        uint8_t i2c_pin = I2C_PIN;
        I2C_DDR |= (1<<I2C_SCL);    // SCL low
        if (i2c_pin & (1 << I2C_SDA))
            data |= 0x01;
        data <<= 1;
    } while (--bits);

    I2C_DDR &= ~(1<<I2C_SDA);       // release SDA
    return data;
}

// start must be called first
uint8_t i2c_write(uint8_t data)
{
    i2c_rw(data);
    scl_hi();

    int err = SUCCESS;
    // NACK = SDA high
    if (I2C_PIN & (1<<I2C_SDA)) err = ADDR_NAK;
    I2C_DDR |= (1<<I2C_SCL);        // SCL low

    return err;
}

uint8_t i2c_start_write(uint8_t addr)
{
    uint8_t err = i2c_start();
    if (err) return err;
    return i2c_write(addr<<1);      // address goes in upper 7 bits
}

uint8_t i2c_start_read(uint8_t addr)
{
    i2c_start();
    return i2c_write(addr<<1 | 1);  // address goes in upper 7 bits
}

uint8_t i2c_restart_read(uint8_t addr)
{
    scl_hi();
    return i2c_start_read(addr);
}

void i2c_stop()
{
    I2C_DDR |= (1<<I2C_SDA);        // SDA low
    I2C_DDR &= ~(1<<I2C_SCL);       // release SCL
    asm ("lpm" ::: "r0");           // 3 cycle delay
    I2C_DDR &= ~(1<<I2C_SDA);       // release SDA
}

// ack !=0 to send ack
static uint8_t i2c_read(uint8_t ack)
{
    uint8_t data = i2c_rw(0xFF);

    if (ack)
        I2C_DDR |= (1<<I2C_SDA);    // SDA low = ACK
    scl_hi();
    asm ("nop");                    // delay
    I2C_DDR |= (1<<I2C_SCL);        // SCL low

    return data; 
}

// ack !=0 to send ack
uint8_t i2c_read_old(uint8_t ack)
{
    // release I2C_SDA
    I2C_DDR &= ~(1<<I2C_SDA);

    uint8_t data = 0, i = 8;
    do{
        asm ("nop");                // delay
        scl_hi();
        uint8_t i2c_pin = I2C_PIN;
        I2C_DDR |= (1<<I2C_SCL);    // SCL low
        if (i2c_pin & (1 << I2C_SDA))
            data |= 0x80;
        data >>= 1;
    } while (--i);

    if (ack)
        I2C_DDR |= (1<<I2C_SDA);    // SDA low = ACK
    scl_hi();
    asm ("nop");                    // delay
    I2C_DDR |= (1<<I2C_SCL);        // SCL low

    return data; 
}

uint8_t i2c_read_ack()
{
    return i2c_read(1);
}

uint8_t i2c_read_nack()
{
    return i2c_read(0);
}

/*
#include "eelog.h"
// weak main for testing
__attribute__ ((weak))\
__attribute__ ((OS_main))\
void main()
{
    // scan bus
    uint8_t addr = 8;
    const uint8_t addr_end = 119;
    do {
        if (i2c_start_write(addr) == 0) {
            // ack received, turn on LED
            PORTB |= (1<<0);    // pullup PB0
            eelog(addr);
        }
        i2c_stop();
        _delay_us(50);          // leave bus idle
    } while (++addr <= addr_end);
    if (PORTB == 0) while (1) {
        // no devices found, flash LED
        PINB |= (1<<0);    // pullup PB0
        _delay_ms(200);
    }
}
*/
