#define BAUD_RATE 57600

/* GPIO for receive */
#define WGMRXBIT    1
#define WGMRXPORT   PORTB
#define WGMRXPIN    PINB

/* GPIO for transmit (OC0A) */
#ifndef OC0A_BIT
#define OC0A_BIT    PB0
#define OC0A_DDR    DDRB
#define OC0A_PORT   PORTB
#endif

#ifndef PCMSK
#define PCMSK PCMSK0
#define PCIE PCIE0
#endif

#ifndef TIMSK
#define TIMSK TIMSK0
#define TIFR TIFR0
#endif

