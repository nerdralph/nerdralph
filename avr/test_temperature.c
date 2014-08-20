/* avr8 temperature reading library
 * http://nerdralph.googlecode.com
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 */

#include <avr/io.h>
#include "temperature.h"


#include "BBUart.h"
#include <stdio.h>
#include <util/delay.h>

FILE uart_output = FDEV_SETUP_STREAM(TxByte, NULL, _FDEV_SETUP_WRITE);

void main(void)
{
    stdout = &uart_output;
    while (1) {
        printf("Temperature= %d\n\r", temperature());
        _delay_ms(1000);
    }
}
