/* avr8 temperature reading library
 * http://nerdralph.googlecode.com
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 * tested with atmega328p
 */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#if defined (REFS1) && !defined(REFS2) && !defined(MUX4)
    // m48 family
    #define ADCINPUT (1<<REFS0) | (1<<REFS1) | (1<<MUX3)
#elif !defined(REFS1) && !defined(MUX4)
    // tinyx8
    #define ADCINPUT (0<<REFS0) | (1<<MUX3)
#elif defined(REFS2)
    // tinyx5 0x0f = MUX0-3
    #define ADCINPUT (0<<REFS0) | (1<<REFS1) | (0x0f)
#elif defined(MUX5)
    // tinyx4 0x0f = MUX0-3
    #define ADCINPUT (0<<REFS0) | (1<<REFS1) | (1<<MUX5) | (1<<MUX1)
#else
    #error unsupported MCU
#endif

uint8_t temp_offset EEMEM;

static uint16_t doADC()
{
    // start conversion with ADC clock prescaler 16
    ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2);               
    while (ADCSRA & (1<<ADSC));         // wait to finish 
    return ADCW;
}

#define ADC_GAIN 1.06
#define SAMPLE_COUNT ((256/ADC_GAIN)+0.5)
// returns signed byte for temperature in oC
char temperature()
{
//    ADMUX = (0<<REFS0) | (1<<MUX3);     // ADC8
    ADMUX = ADCINPUT;
    uint16_t tempRaw = 0;
    // take multiple samples then average 
    for (uint8_t count = SAMPLE_COUNT; --count;) {
        tempRaw += (doADC() - 273);    
    }
    ADCSRA = 0;                         // turn off ADC

    // a known offset could be used instead of the calibrated value
    return ((tempRaw/256) - eeprom_read_byte(&temp_offset) ) ;
}

// temperature at programming time
#define AIR_TEMPERATURE 25
//__attribute__ ((constructor)) void calibrate_temp (void)
// using section .init8 makes for smaller code than constructor
__attribute__ ((naked))\
__attribute__ ((used))\
__attribute__ ((section (".init8")))\
void calibrate_temp (void)
{
    if ( eeprom_read_byte(&temp_offset) == 0xff)
    {
        // temperature uncalibrated
        char tempVal = temperature();   // throw away 1st sample
        tempVal = temperature();
        // 0xff == -1 so final offset is reading - AIR_TEMPERATURE -1
        eeprom_write_byte( &temp_offset, (tempVal - AIR_TEMPERATURE) -1);
    }
}

