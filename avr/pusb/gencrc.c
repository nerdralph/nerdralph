// Ralph Doncaster 2020
// crc16 pre-calculator for host

#include <stdio.h>
#include <stdint.h>

extern const char DDesc[];
extern const char CDesc[];

#if 0
// device descriptor
const char DDesc[] = {
    18 + 1,                 // sizeof(descriptor): length in bytes + salt
    1,                      // device descriptor type
    0x10, 0x01,             // USB version supported
    0,                      // device class
    0,                      // device subclass
    0,                      // protocol
    8,                      // max packet size
    (char)USB_CFG_VENDOR_ID,// 2 bytes
    (char)USB_CFG_DEVICE_ID,// 2 bytes
    USB_CFG_DEVICE_VERSION, // 2 bytes
    0,                      // manufacturer string index
    0,                      // product string index
    0,                      // serial number string index
    1,                      // number of configurations
    0,                      // CRC salt padding
};


const char CDesc[] = {
    9,                      // sizeof(descriptor): length in bytes
    2,                      // config descriptor type
    9, 0,                   // total length of data returned
    0,                      // number of interfaces in this config
    1,                      // index of this configuration
    0,                      // configuration name string index
    (1 << 7),               // attributes bus powered */
    USB_CFG_MAX_BUS_POWER/2,    // max USB current in 2mA units
};
#endif

uint32_t crc16_8(const char* block, uint8_t len)
{
    const uint32_t poly = 0xA001;
    uint32_t crc = 0;
    uint8_t count = len;
    do {
        uint8_t bits = 8;
        crc ^= (uint8_t)(*block++);
        do {
            int lsbit = crc & 0x0001;
            crc = (crc >> 1) | 0x8000;
            if (lsbit == 0)
                crc ^= poly;
        } while (--bits);
    } while (--count);
    return crc;
}

int main()
{
    uint32_t crc16 = crc16_8(&DDesc[0], 8);
    printf ("DDCRC1 0x%X\n", crc16);
    crc16 = crc16_8(&DDesc[10], 8);
    printf ("DDCRC2 0x%X\n", crc16);
    crc16 = crc16_8(&DDesc[20], 3);
    printf ("DDCRC3 0x%X\n", crc16);

    crc16 = crc16_8(&CDesc[0], 8);
    printf ("CDCRC1 0x%X\n", crc16);
    crc16 = crc16_8(&CDesc[10], 1);
    printf ("CDCRC2 0x%X\n", crc16);
}
