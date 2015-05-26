/* rf diagnostics
 * detect and diagnose connections for nrf24l01, SE8R01, & Si24R1
 * @author: Ralph Doncaster 2015
 */

#include "spi.h"
#include "tinyWiring.h"

void main()
{
    spi_setup();
    spi_byte(0);
}

