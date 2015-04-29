/* Ralph Doncaster 2015 public domain software
 * small binary to ascii code
 */

// converts 4-bit nibble to ascii hex
uint8_t nibbletohex(uint8_t value) 
{
    if ( value > 9 ) value += 'A' - '0';
    return value + '0';
}

// returns value as 2 ascii characters in a 16-bit int
uint16_t u8tohex(uint8_t value)
{
    uint16_t hexdigits;
    
    uint8_t hidigit = (value >> 4);
    hexdigits = (nibbletohex(hidigit) << 8);
 
    uint8_t lodigit = (value & 0x0F);
    hexdigits |= nibbletohex(lodigit);

    return hexdigits;
}
