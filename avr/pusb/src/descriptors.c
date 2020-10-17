// Ralph Doncaster 2020
// descriptors for picoUSB

#include <stdint.h>
#include "config.h"

// todo: define DeviceDesc struct
// combined device and configuration descriptor 
const uint8_t DDesc[] = {
    // device descriptor
    18 + 1,                 // sizeof(descriptor): length in bytes + salt
    1,                      // device descriptor type
    0x10, 0x01,             // USB version supported
    0,                      // device class
    0,                      // device subclass
    0,                      // protocol
    8,                      // max packet size
    0xD0, 0xBB,             // CRC16
    0xD0, 0x16,             // vendor
    0x53, 0x07,             // device
    0x6, 0x6,               // version
    0,                      // manufacturer string index
    0,                      // product string index
    0x9C, 0x43,             // CRC16
    0,                      // serial number string index
    1,                      // number of configurations
    0,                      // CRC salt padding
    0x8F, 0xAF,             // CRC16
};

const uint8_t CDesc[] = {
    // configuration descriptor
    9,                      // sizeof(descriptor): length in bytes
    2,                      // config descriptor type
    9, 0,                   // total length of data returned
    0,                      // number of interfaces in this config
    1,                      // index of this configuration
    0,                      // configuration name string index
    (1 << 7),               // attributes bus powered */
    0x0C, 0xA7,             // CRC16
    100/2,                  // max USB current in 2mA units
    0xC1, 0x6A,             // CRC16
};
