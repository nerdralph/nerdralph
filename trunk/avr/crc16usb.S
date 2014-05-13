/* size optimized CRC-16 for 8-byte USB packet - 17 instructions
 * uses avr-libc calling convention
 * takes pointer argument in r25:r24
 * min/max speed: 56/64 cycles/byte
 * returns CRC in r25:r24
 * @author: Ralph Doncaster 2014
 */

#include <avr/io.h>

#define tmp r0
#define polyH r19
#define polyL r18
#define data r20
#define count r20
#define crcH r25
#define crcL r24

.global crc16usb
crc16usb:
    ; movw not needed if pointer passed in X
    movw XL, r24
    clr crcH
    clr crcL
    ldi polyH, 0xA0
    ldi polyL, 0x01
    clr count
nextByte:
    ld data, X+
    eor crcL, data
nextBit:
    lsr crcH
    ror crcL
    brcc noXor
    eor crcH, polyH
    eor crcL, polyL
noXor:
    subi count, -2              ; add 2
    brhs nextBit                ; half-carry clr every 8 bits
    brpl nextByte               ; N bit set after 8 bytes
    ret
