/* BBUart example program
 * compile with -DBAUD_RATE=57600 to use 57.6kbps
 * see BBUart.h for default when BAUD_RATE is undefined
 */ 

#include "BBUart.h"

const unsigned char EOF = 0x04;

void main(void)
{
    unsigned char c = ':';
    TxByte(c);
    while ( (c = RxByte()) != EOF ){
        TxByte(c);
    }
}

