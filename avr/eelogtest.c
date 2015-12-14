#include "eelog.h"

void main(void)
{
    volatile uint8_t* ioreg;
    uint8_t offset = 0x3f;
    eelog(42);
    ioreg = (uint8_t*)0x0020;
    do {
        eelog(*(ioreg + offset));
    } while ( offset-- );
}

