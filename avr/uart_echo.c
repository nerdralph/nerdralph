#define BAUD_RATE 38400
#include "BBUart.h"

const unsigned char EOF = 0x04;

void main(void)
{
    unsigned char val = 42;
    while ( (val = RxByte()) != EOF ){
        TxByte(val);
    }
}

