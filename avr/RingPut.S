
; 1 arg: data to add to ring buffer in r18
; clobbers r30, r31
; 30 bytes of flash used
.global RingPut
RingPut:
    lds r30, gRingBuf                   ; head
    lds r31, gRingBuf+1                 ; tail
    cp r30, r31
    breq 1f                             ; buffer full
    inc r30
    andi r30, 0x1F
    sts gRingBuf, 30
    andi r30, 0x0F
    subi r30, lo8(-(gRingBuf))          ; add buffer address
    sbci r31, hi8(-(gRingBuf))
    std Z+2, r18
1:  ret
