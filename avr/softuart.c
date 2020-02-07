#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef PU_BAUD_RATE
#define PU_BAUD_RATE 115200L            // default baud rate
#endif

#ifndef PU_TX
#define PU_TX B,1
#define PU_RX B,0
#endif

// use static inline functions for type safety
extern inline float PUBIT_CYCLES() {return F_CPU/(PU_BAUD_RATE*1.0);}

// delay based on cycle count of asm code + 0.5 for rounding
extern inline int PUTXWAIT() {return PUBIT_CYCLES() - 7 + 0.5;}
extern inline int PURXWAIT() {return PUBIT_CYCLES() - 5 + 0.5;}

// correct for PURXWAIT skew in PURXSTART calculation
// skew is half of 7 delay intervals between 8 bits
extern inline float PUSKEW() {
    return (PUBIT_CYCLES() - (int)(PUBIT_CYCLES() + 0.5)) * 3.5;
}
// Time from falling edge of start bit to sample 1st bit is 1.5 *
// bit-time. Subtract 2 cycles for sbic, 1 for lsr, and PURXWAIT.
// Subtract 1.5 cycles because start bit detection is accurate to
// +-1.5 cycles.  Add 0.5 cycles for int rounding, and add skew.
extern inline int PURXSTART() {
    return (PUBIT_CYCLES()*1.5 -3 -PURXWAIT() -1 +PUSKEW());
}

// min rx/tx turn-around time in resistor-only 1-wire mode
inline void pu_rxtx_wait()
{
    __builtin_avr_delay_cycles(PUBIT_CYCLES()*1.5);
}

// I/O register macros
#define GBIT(r,b)       (b)
#define GPORT(r,b)      (PORT ## r)
#define GDDR(r,b)       (DDR ## r)
#define GPIN(r,b)       (PIN ## r)
#define get_bit(io)     GBIT(io)
#define get_port(io)    GPORT(io)
#define get_ddr(io)     GDDR(io)
#define get_pin(io)     GPIN(io)
#define PUTXBIT     get_bit(PU_TX)
#define PUTXPORT    get_port(PU_TX)
#define PUTXDDR     get_ddr(PU_TX)

typedef union {
    unsigned i16;
    struct { uint8_t lo8; uint8_t hi8; };
} frame;

typedef union {
    uint8_t c;
    struct {
         int b0:1; int b2:1; int b3:1; int b4:1; int b5:1; int b6:1; int b7:1;
    };
} bits;

void tx(uint8_t c)
{
    //register frame f asm("r24");
    frame f;
    f.lo8 = c;
    PUTXPORT &= ~(1<<PUTXBIT);          // disable pullup
    cli();
    PUTXDDR |= (1<<PUTXBIT);            // low for start bit
    // hi8 b1 set for stop bit, b2 set for line idle state
    f.hi8 = 0x03;
    register bits psave asm ("r0") = {PUTXPORT};
    // do {
    txbit:
        __builtin_avr_delay_cycles(PUTXWAIT());
        psave.b2 = f.lo8 & 0x01 ? 1 : 0;
        f.i16 >>= 1;
        PUTXPORT = psave.c; 
    // tx more bits if f.lo8 not equal to 0
    asm goto ("brne %l[txbit]" :::: txbit);
    //} while (f.lo80);
    sei();
    PUTXDDR &= ~(1<<PUTXBIT);            // revert to input mode
}

void prints(const char* s)
{
    char c;
    while(c = *s++) tx(c);
}

void main()
{
    prints("picoUART-C\r\n");
}

