/* avr eeprom debug logging
 * http://nerdralph.blogspot.ca
 * @author: Ralph Doncaster 2015
 * define ISR_SAFE before including eelog.h if interrupts are enabled
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

void eelog(char logdata)
{
    eeprom_busy_wait();
    EEARL = (EEARL + 1);
    EEDR = logdata;
#ifdef ISR_SAFE
    cli();
#endif
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
#ifdef ISR_SAFE
    sei();
#endif
}

__attribute__ ((naked))\
__attribute__ ((used))\
__attribute__ ((section (".init8")))\
void eelog_init(void)
{
    EEARL = 0xff;
#ifdef EEARH
    EEARH = 0;
#endif

}

