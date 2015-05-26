/* Wiring for attinyx5 & attiny13
 * @author: Ralph Doncaster 2015
 */

#include <avr/io.h>

#define NOT_A_REG 0

#define digitalPinToPortReg(PIN) \
    ( ((PIN) >= 0 && (PIN) <= 7) ? &PORTB : NOT_A_REG)

#define digitalPinToBit(P) ((P) & 7)

#define HIGH 1
#define LOW 0

inline void digitalWrite(int pin, int state)
{
    if ( state & 1 ) {    
        // set pin
        *(digitalPinToPortReg(pin)) |= (1 << digitalPinToBit(pin));
    }
    if ( !(state & 1 ) ) {    
        // clear pin
        *(digitalPinToPortReg(pin)) &= ~(1 << digitalPinToBit(pin));
    }
}
