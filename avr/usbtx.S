; Ralph Doncaster 2020
; prototype 7 cycle/bit 10.5MHz USB transmit

#define __SFR_OFFSET 0
#include <avr/io.h>

#define USBDM       0
#define USBDP       1
#define USBPORT     PORTB
#define USBPID_ACK  0xD2
#define USB_SYNC    0x80
#define USBMSK      (1<<USBDM) | (1<<USBDP)

.data
txBuf:
; PID + <= 8B data + CRC16 + <= 13 stuffed bits
.skip 1 + 8 + 2 + 2

#define data r18
#define next r19
#define cnt r20
#define mask r21
#define psave r22
#define nop2 rjmp .

.text
.globl main
main:
    ; want 10.5Mhz
    in r24, OSCCAL
    subi r24, -7
    out OSCCAL, r24
again:
    ldi XL, lo8(txBuf)
    ldi XH, hi8(txBuf)
    ldi data, USB_SYNC
    ldi r24, USBPID_ACK
    st x, r24
    ldi cnt, 256 - 2*16
    ; in psave, USBPORT
    clr psave
    ori psave, 1<<USBDM                 ; J/idle state
    out USBPORT, psave
    ldi mask, USBMSK
    out USBPORT-1, mask                 ; set DDR
    rjmp tx2bit

; 7-cycle/bit tx loop, cnt = 2 * # of bits to tx
nextByte:
    ld data, X+
    breq txEoP
tx2bit:
    sbrs data, 0
    out USBPORT-2, mask
    lpm
    lsr data
    subi cnt, -4
    sbrs data, 0
    out USBPORT-2, mask
1:
    brhc nextByte   ; subi modifies H
    lsr data
    nop
    rjmp tx2bit
    ; todo:  check for last bit to tx
    ; breq txEoP
    ; sbrs data, 1
    ; eor psave, mask
    ; out USBPORT, psave

txEoP:
    out USBPORT, r1
    rcall doRet
    nop2
    nop2
    andi psave, ~(USBMSK)
    ori psave, 1<<USBDM                 ; J/idle state
    out USBPORT, psave
delay:
    sbiw Yl, 1
    brne delay
    rjmp again
doRet:
    ret
