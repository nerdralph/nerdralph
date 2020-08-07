#include <stdint.h>

#define BUFSIZE2N 4
#if BUFSIZE2N > 7
#error "2exp(BUFSIZE2N) must be <= 256"
#endif

typedef struct {
    uint8_t head;
    uint8_t tail;
    uint8_t buf[1<<BUFSIZE2N];
} RingBuf;

//extern RingBuf gRingBuf;
__attribute(( section(".noinit") ))
RingBuf gRingBuf;

void RingPut(uint8_t val)
{
    uint8_t size = (1<<BUFSIZE2N);
    if (gRingBuf.head == gRingBuf.tail)
        return;                         // full
    gRingBuf.buf[gRingBuf.head & (size -1)] = val;
    gRingBuf.head = (gRingBuf.head + 1) & (size * 2 - 1);
}

inline uint8_t RingDiff()
{
    char diff = gRingBuf.head - gRingBuf.tail;
    // ternary operator results in integer promotion
    //return (diff >= 0) ? diff : -diff;
    if (diff >= 0) return diff;
    else return -diff;
}

/*
static char RingCanGet()
{
    return RingDiff() - (1<<BUFSIZE2N);
}
*/

// RingCanGet() should be checked first; no error checking here
int RingGet()
{
    uint8_t size = (1<<BUFSIZE2N);
    // cast doesn't stop integer promotion
    //if (abs ( (char)(gRingBuf.head - gRingBuf.tail) ) == size )
    // abs == nop with & 0xFF
    //if (abs ( (gRingBuf.head - gRingBuf.tail) & 0xFF) == size )
    //char diff = gRingBuf.head - gRingBuf.tail;
    //if (diff == size || diff == -size)
    //    return -1;                       // empty
     
    int val;
    //if ( ! RingCanGet() ) val = -1; 
    if ( RingDiff() == size )
        val = -1;                       // empty
    else { 
        gRingBuf.tail = (gRingBuf.tail + 1) & (size * 2 - 1);
        val = gRingBuf.buf[gRingBuf.tail & (size - 1)];
    }
    return val;
}

void RingInit()
{
    gRingBuf.tail = 0;
    gRingBuf.head = (1<<BUFSIZE2N);
}

volatile uint8_t testcount;

int main()
{
    RingInit();
    uint8_t end = testcount;
    for (uint8_t i = 0; i <= end; i++) RingPut(i);
    // try counting RingGet until -1 to avoid optimizer
    //RingPut(42);
    return RingGet();
}
