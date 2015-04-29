
#include <avr/io.h>
#include "u8tohex.h"

int main()
{
    int result = u8tohex(GPIOR0);
    result += u8tohex(GPIOR0);
    return result;
}
