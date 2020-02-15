/* avr eeprom debug logging
 * http://nerdralph.blogspot.ca
 * @author: Ralph Doncaster 2015
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

void eelog(char logdata)
{
    while (!eeprom_is_ready());
    // EEARL can't be changed while eeprom busy
    EEARL = (EEARL + 1);
    EEDR = logdata;

    // ISR could occur before EEPE is set so repeat until it worked
    do {
        EECR |= (1<<EEMPE);
        EECR |= (1<<EEPE);
    } while (eeprom_is_ready());
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

