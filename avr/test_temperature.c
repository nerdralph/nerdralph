/* avr8 temperature reading library
 * @author: Ralph Doncaster 2014
 * @version: $Id$
 * works with attinyx8, atmegaxx8
 */

#include <avr/io.h>
#include "temperature.h"

void main()
{
    GPIOR0 = temperature();
}
