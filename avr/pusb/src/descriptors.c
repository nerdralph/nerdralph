// Ralph Doncaster 2020
// descriptors for picoUSB

#include <stdint.h>
#include "config.h"

// todo: define DeviceDesc struct
// device descriptor
const uint8_t DDesc[] = {
    18 + 1,                 // sizeof(descriptor): length in bytes + salt
    1,                      // device descriptor type
    0x10, 0x01,             // USB version supported
    0,                      // device class
    0,                      // device subclass
    0,                      // protocol
    8,                      // max packet size
    0xD0, 0x16,             // vendor
    0x53, 0x07,             // device
    0x6, 0x6,               // version
    0,                      // manufacturer string index
    0,                      // product string index
    0,                      // serial number string index
    1,                      // number of configurations
    0,                      // CRC salt padding
};


// configuration descriptor
const uint8_t CDesc[] = {
    9,                      // sizeof(descriptor): length in bytes
    2,                      // config descriptor type
    9, 0,                   // total length of data returned
    0,                      // number of interfaces in this config
    1,                      // index of this configuration
    0,                      // configuration name string index
    (1 << 7),               // attributes bus powered */
    100/2,                  // max USB current in 2mA units
};
