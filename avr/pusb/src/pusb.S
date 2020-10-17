; Ralph Doncaster 2020
; prototype 7 cycle/bit 10.5MHz USB transmit
; tx =~ 40 instruction, rx =~ 35 w/o address match

#define __SFR_OFFSET 0
#include <avr/io.h>

#define USBDM       0
#define USBDP       1
#define USBPORT     PORTB
#define PID_SETUP   0x2D
#define PID_ACK     0xD2
#define USB_SYNC    0x80
#define USBMSK      (1<<USBDM) | (1<<USBDP)

#define USBDDR      (USBPORT-1)
#define USBIN       (USBPORT-2)

.data
txBuf:
; PID + <= 8B data + CRC16
.skip 1 + 8 + 2
rxBuf:
; PID + 8B data, CRC16 ignored
.skip 1 + 8


;DDesc:
;.byte   18          ; length
;.byte   1           ; descriptor type
;.word   0x0110      ; USB version 1.1

#define usbAddr r2
#define txLen r16
#define data r18
#define cnt r19
#define mask r20
#define psave r21
#define even r22
#define odd r23
#define nop2 rjmp .

.text
.extern DDesc
.extern CDesc

error:
    rjmp error

wait10c:
    ldi r19, 1
; wait 3 cycles times r19
; rcall/ret overhead adds 7 cycles
wait3c_r19:
    dec r19
    brne wait3c_r19
ret

; USB bus reset is SE0 for > 3 bit-times
awaitReset:
    ldi r24, 8                          ; want SE0 8 times
awaitSE0:
    mov cnt, r24                        ; r24 = # of SE0 samples
1:
    in psave, USBIN
    andi psave, USBMSK
    brne awaitSE0
    dec cnt
    brne 1b
ret:
ret

; copy descriptor chunk <= 10B from flash to RAM
cpDescrChunk:
    ldi cnt, 10
    cpi txLen, 10                       ; max 8B + 2B CRC
    brge 1f
    mov cnt, txLen
1:
    sub txLen, cnt
    ldi XL, lo8(txBuf)
    ldi XH, hi8(txBuf)
copy:
    lpm r0, Z+
    st X+, r0
    dec cnt
    brne copy
ret


#define bitcnt cnt
; wait for Token packet
getToken:
    ldi bitcnt, 255 - 2                 ; read 2 bytes after sync
    rcall awaitPacket
    ldi r24, 2
    rcall awaitSE0                      ; wait for EOP
    cp data, usbAddr
    brne getToken                       ; no address match
    ld data, -X                         ; get PID
    cpi data,  PID_SETUP
    brne error
    ldi bitcnt, 255 - 8                 ; read 8 byte setup request
    rcall awaitPacket
    sbiw XL, 7                          ; rewind X to rxBuf
    

; wait for USB communication, idle = J, USBDM high & USBDP low
; all packets start with sync: KJKJKJKK, then PID
awaitPacket:
    sbic USBIN, USBDM
    rjmp awaitPacket
    rcall ret                           ; 7c delay
    sbis USBIN, USBDM                   ; got J
    rjmp checkSE0
awaitK:
    ; 11-14 cycles after 1st JK transition
    sbis USBIN, USBDM
    rjmp gotK
    sbis USBIN, USBDM
    rjmp gotK
    sbis USBIN, USBDM
    rjmp gotK
    ; check for keepalive or bus reset
checkSE0:
    rcall wait10c
    in psave, USBIN
    andi psave, USBMSK
    brne awaitPacket                    ; skip keepalive
    clr usbAddr                         ; got bus reset
    ldi ZL, lo8(DDesc)
    ldi ZH, hi8(DDesc)
    rcall awaitPacket

gotK:
    ; 3-5 cycles after JK transistion on 1st round
    ldi XL, lo8(rxBuf)                  ; 3
    ldi XH, hi8(rxBuf)                  ; 4
#define numBytes 2
    ldi bitcnt, 255 - numBytes          ; 5
    nop2                                ; 6
    nop2                                ; 8
    sbic USBIN, USBDM                   ; 10 - sample bit
    rjmp awaitK                         ; 11 + 2c = 13cycle loop

    ; now 12c after start of KK, sample 1st bit at 18th cycle
    lpm $ lpm                           ; 12 + 6c = 18c for next opcode
rxData:
    ; 2 bits unrolled * 3, + 6/7 unrolled
    ; ldi bitcnt, 255 - numBytes          ; 85 * 3 = 255
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
    ; fall through for rx complete
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
    ldi r24, PID_ACK
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

