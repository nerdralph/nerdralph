/* avr8 temperature reading library
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 * works with attinyx8, atmegaxx8
 */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>

uint8_t temp_offset EEMEM;

static uint16_t doADC()
{
    // start conversion with ADC clock prescaler 16
    ADCSRA = (1<<ADSC) | (1<<ADPS2);               
    while (ADCSRA && (1<<ADSC));        // wait to finish 
    return ADCW;
}

#define ADC_GAIN 1.06
#define SAMPLE_COUNT ((256/ADC_GAIN)+0.5)
/* returns signed byte for temperature in oC */
char temperature()
{
    ADMUX = (0<<REFS0) | (1<<MUX3);     // ADC8
    uint16_t tempRaw = 0;
    // take multiple samples then average 
    for (uint8_t count = SAMPLE_COUNT; --count;) {
        tempRaw += (doADC() - 273);    
    }
    return ((tempRaw/256) - eeprom_read_byte(&temp_offset) ) ;
}
