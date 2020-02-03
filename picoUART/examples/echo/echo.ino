// picoUART echo test

// override default baud rate
#define PU_BAUD_RATE 230400 

#include <picoUART.h>
#include <avr/pgmspace.h>

void setup() {}

void loop()
{
    prints_P(PSTR("\r\npicoUART echo "));
    // read then echo 1 character
    char c = pu_rx();
    pu_tx(c);
}
