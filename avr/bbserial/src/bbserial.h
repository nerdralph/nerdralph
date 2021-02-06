#pragma once

#include <stdint.h>

#define DIVIDE_ROUNDED(NUMERATOR, DIVISOR) ((((2*(NUMERATOR))/(DIVISOR))+1)/2)

#define BIT_CYCLES(baud) DIVIDE_ROUNDED(F_CPU,baud) 

extern uint16_t CyclesPerBit;

extern void WriteB(uint8_t);

