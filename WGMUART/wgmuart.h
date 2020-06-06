#define WGMRXBIT 1

#ifndef OC0A_BIT
#define OC0A_BIT PB0
#define OC0A_DDR DDRB
#define OC0A_PORT PORTB
#endif

#ifdef TIMSK0
#define TIMSK TIMSK0
#define TIFR TIFR0
#endif

