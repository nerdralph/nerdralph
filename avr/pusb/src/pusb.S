; Ralph Doncaster 2020
; prototype 7 cycle/bit 10.5MHz USB transmit
; tx =~ 40 instruction, rx =~ 35 w/o address match

#define __SFR_OFFSET 0
#include <avr/io.h>

#define USBDM       0
#define USBDP       1
#define USBPORT     PORTB
#define USBPID_ACK  0xD2
#define USB_SYNC    0x80
#define USBMSK      (1<<USBDM) | (1<<USBDP)

#define USBDDR      (USBPORT-1)
#define USBIN       (USBPORT-2)

.data
txBuf:
; PID + <= 8B data + CRC16
.skip 1 + 8 + 2

;DDesc:
;.byte   18          ; length
;.byte   1           ; descriptor type
;.word   0x0110      ; USB version 1.1

#define data r18
#define cnt r19
#define mask r20
#define psave r21
#define nop2 rjmp .

.text
.extern DDesc
.extern CDesc

; wait 3 cycles times r19
; rcall/ret overhead adds 7 cycles
wait3c_r19:
    dec r19
    brne wait3c_r19
ret

; USB bus reset is SE0 for > 3 bit-times
waitReset:
    ldi cnt, 8                          ; want SE0 8 times
1:
    in psave, USBIN
    andi psave, USBMSK
    brne waitReset
    dec cnt
    brne 1b
ret

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
    sbi USBPORT, USBDM                  ; J/idle state
    ldi mask, USBMSK
    in r0, USBPORT-1
    or r0, mask
    out USBDDR, r0                      ; set DDR
    rjmp tx2bit

; 7-cycle/bit tx loop, cnt = 2 * # of bits to tx
nextByte:
    breq txEoP
    ld data, X+
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

txEoP:
    in data, USBPORT
    cbr data, USBMSK
    out USBPORT, data 
    rcall doRet
    lpm
    nop2
    sbi USBPORT, USBDM                  ; J/idle state
    cbi USBPORT-1, USBDP                ; input mode
    cbi USBPORT-1, USBDM                ; input mode
    ; could leave D- pullup enabled?
    cbi USBPORT, USBDM                  ; disable pullup
delay:
    sbiw Yl, 1
    brne delay
    rjmp again
doRet:
    ret

#define cnt7bit r22
v2tx:
    sbrs data, 0
    out USBPORT-2, mask
    nop
    lsr data
    subi cnt7bit, 37                    ; 256/7 = 36.57 
    brcc v2tx
    dec cnt
    sbrs data, 0
    out USBPORT-2, mask
    ld data, X+
    nop
    brne v2tx

; doesn't require PIN reg write for toggle, 12 instr
v3tx:
    sbrs data, 0
    eor psave, mask
    out USBPORT, psave                  ; bits 0-6
    lsr data
    subi cnt7bit, 37                    ; 256/7 = 36.57 
    brcc v2tx
    dec cnt
    sbrs data, 0
    eor psave, mask
    out USBPORT, psave                  ; bit 7
    ld data, X+
    brne v2tx

; half-C cnt not limited to 11 byte like cnt7bit, 12 instr
; will fail due to no subi cnt for bit 7
v4tx:
    sbrs data, 0
    eor psave, mask
    out USBPORT, psave                  ; bits 0-6
    lsr data
    subi cnt, -2
    brhs v4tx
    sbrs data, 0
    eor psave, mask
    tst cnt
    out USBPORT, psave                  ; bit 7
    ld data, X+
    brne v2tx

#if 0
; 2 bits unrolled * 3, + 6/7 unrolled
    ldi bitcnt, 255 - numBytes          ; 85 * 3 = 255
rxbit:
    in even, USBIN                      ; bits 2 & 4
    rjmp 1f
rxSave:
    in even, USBIN                      ; bit 0
    st X+, data
1:  nop
    eor odd, even
    subi odd, 1
    ror data

    in odd, USBIN                       ; bits 1, 3 & 5
    eor even, odd
    subi even, 1
    ror data
    subi bitcnt, 85                     ; 85 * 3 = 255
    brcc rxbit

    in even, USBIN                      ; bit 6
    eor odd, even
    subi odd, 1
    ror data
    lpm                                 ; 3c delay

    in odd, USBIN                       ; bit 7
    eor even, odd
    subi even, 1
    ror data
    cpi bitcnt, -1
    brne rxSave
    ; fall through for rx complete, wait for EOP
#endif

