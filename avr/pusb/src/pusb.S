; Ralph Doncaster 2020
; prototype 7 cycle/bit 10.5MHz USB transmit

#define __SFR_OFFSET 0
#include <avr/io.h>

#define USBDM       0
#define USBDP       1
#define USBPORT     PORTB
#define USB_SYNC    0x80
#define USBMSK      (1<<USBDM) | (1<<USBDP)
#define USBDDR      (USBPORT-1)
#define USBIN       (USBPORT-2)

// USB packet IDs
#define PID_SETUP   0x2D
// PID_SETUP = 0x2D
#define PID_OUT     0xE1
#define PID_IN      0x69
#define PID_ACK     0xD2
#define PID_DATA0   0xC3
#define PID_DATA1   0x4B

#define SET_ADDRESS       5
#define GET_DESCRIPTOR    6

.data
; txBuf:
; PID + <= 8B data + CRC16
; .skip 1 + 8 + 2
rxBuf:
; PID + 8B data, CRC16 ignored
.skip 1 + 8

.text
.extern DDesc
.extern CDesc


usbAck:
.byte PID_ACK, 0                        ; 0 pad for 2-byte alignment

; register assignments
; todo: differentiate local vs global registers

usbAddr = 2
newAddr = 3
usbPID  = 4
tmpr5   = 5
txLen   = 16
data    = 18
cnt     = 19
mask    = 20                            ; USB D+/D- mask
psave   = 21                            ; port save

#define nop2    rjmp .

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

#if 0
; cpDescrChunk no longer needed with direct tx from flash
; copy descriptor chunk <= 10B from flash to RAM & toggle DATA PID
; args: cnt, Z
; caller sets Z to descriptor address in flash
; caller sets cnt to number of bytes to copy
cpDescrChunk:
    ldi cnt, 10
    cpi txLen, 10                       ; max 8B + 2B CRC
    brge 1f
    mov cnt, txLen
1:
    sub txLen, cnt
    ldi XL, lo8(txBuf)
    ldi XH, hi8(txBuf)
    ldi data, PID_DATA0 ^ PID_DATA1
    eor usbPID, data
    st X+, usbPID
copy:
    lpm r0, Z+
    st X+, r0
    dec cnt
    brne copy
ret
#endif 

; wait for Token packet - IN, OUT, or SETUP
getToken:
    ldi cnt, 2                          ; read 2 bytes after sync
    rcall readPacket
    ldi r24, 2
    rcall awaitSE0                      ; wait for EOP
    cp data, usbAddr
    brne getToken                       ; no address match
    ld data, -X                         ; get PID
    cpi data,  PID_SETUP
    brne pidIn                          ; check for IN/OUT
    ldi cnt, 9                          ; want 9 bytes
    rcall readPacket                    ; read setup request
    sbiw XL, 6                          ; rewind X to bRequest
    ld data, X+
    cpi data, GET_DESCRIPTOR
    brne setAddress
    ldi ZL, lo8(DDesc)
    ldi ZH, hi8(DDesc)
setAddress:
    cpi data, SET_ADDRESS
    brne userFunction
    ld newAddr, X
    ; reply zero-len descriptor
pidIn:
    cpi data,  PID_IN
    brne pidOut
pidOut:
userFunction:
    rjmp error

; register allocations for readPacket
even    = 22
odd     = 23
; wait for USB communication, idle = J, USBDM high & USBDP low
; all packets start with sync: KJKJKJKK, then PID
; read cnt bytes into rxBuf, X points to last byte on return
readPacket:
    sbic USBIN, USBDM
    rjmp readPacket
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
    brne readPacket                     ; skip keepalive
    clr usbAddr                         ; got bus reset
    rcall readPacket

gotK:
    ; 3-5 cycles after JK transistion on 1st round
    ldi XL, lo8(rxBuf)                  ; 3
    ldi XH, hi8(rxBuf)                  ; 4
    mov tmpr5, cnt                      ; 5
    nop2                                ; 6
    nop2                                ; 8
    sbic USBIN, USBDM                   ; 10 - sample bit
    rjmp awaitK                         ; 11 + 2c = 13cycle loop

    ; now 12c after start of KK, sample 1st bit at 18th cycle
    lpm $ lpm                           ; 12 + 6c = 18c for next opcode
rxData:
    ; 2 bits unrolled * 3, + 6/7 unrolled
    in even, USBIN                      ; bits 2 & 4
    rjmp 1f
rxSave:
    in even, USBIN                      ; bit 0
    st X+, data
1:  nop
    eor odd, even
    subi odd, 1                         ; set C if last 2 bits are the same
    ror data

    in odd, USBIN                       ; bits 1, 3 & 5
    eor even, odd
    subi even, 1
    ror data
    ; 85 * 3 = -1, so cnt decreases by 1 for every byte 
    subi cnt, -85
    brcs rxData

    in even, USBIN                      ; bit 6
    eor odd, even
    subi odd, 1                         ; set C if last 2 bits are the same
    ror data
    lpm                                 ; 3c delay

    in odd, USBIN                       ; bit 7
    eor even, odd
    subi even, 1
    ror data
    tst cnt
    brne rxSave
    ; fall through for rx complete
ret

.globl main
main:
    ; want 10.5Mhz
    in r24, OSCCAL
    subi r24, -7
    out OSCCAL, r24

    ldi r24, PID_DATA0
    mov usbPID, r24

; txPacket register assignments
; todo: use txSync, then load data with PID
cnt7bit = 22
byteCnt = 19
txACK:
    ldi ZL, lo8(usbAck)
    ldi ZH, hi8(usbAck)
    ldi byteCnt, 1

; transmit packet of cnt bytes in flash pointed to by Z
txPacket:
    ldi data, USB_SYNC
    sbi USBPORT, USBDM                  ; J/idle state
    ldi mask, USBMSK
    in r0, USBDDR
    or r0, mask
    out USBDDR, r0                      ; set DDR

; 7-cycle/bit tx loop, 13 instr
    ldi cnt7bit, 255                    ; good for 37/3 == 12 loops/bytes
txBit:
    nop                                 ; [5]
txByte:
    sbrs data, 0                        ; [6]
    out USBIN, mask                     ; [7] write bits 0-6
    lsr data                            ; [1]
    subi cnt7bit, 37                    ; [2] 256/7 = 36.57 
    brcc txBit                          ; [3]
    nop                                 ; [4]
    dec byteCnt                         ; [5]
    sbrs data, 0                        ; [6]
    out USBIN, mask                     ; [7] write bit 7
    lpm data, Z+                        ; [1]
    brpl txByte                         ; [4]

    ; now 3 cycles into last bit
txEoP:
    nop                                 ; [4] 
    ldi r19, 2                          ; [5] for wait3c_r19
    cbr psave, USBMSK                   ; [6] 
    out USBPORT, psave                  ; [7] SE0 for 2bit = 14c
    rcall wait3c_r19                    ; [1] 12c delay
    sbi USBPORT, USBDM                  ; [13] J/idle state on 2nd cycle
    cbi USBDDR, USBDP                   ; input mode
    cbi USBDDR, USBDM                   ; input mode
    ; could leave D- pullup enabled?
    cbi USBPORT, USBDM                  ; disable pullup
    ret

#if 0
    ldi bitcnt, 7
txSync:
    out USBPORT, mask                   ; toggle D+ & D-
    lpm                                 ; 3c delay
    dec bitcnt
    brne txSync
#endif
