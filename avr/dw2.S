; digitalWrite in 31 instr / 62 bytes
#define __SFR_OFFSET 0
#include <avr/io.h>

#define bitval r25
#define pin r24
#define pinstate r22

; ~21 + 18 = 39 cycles avg execution time, 50 bytes
; pin state modification is atomic
dWrite:
ldi ZL, PINB
clr ZH
cpi pin, 8
brcc 1f
; PORTD
subi ZL, -6  ; PORTD = PORTB + 6
rjmp _modpin
1:
cpi pin, 14
brcc 2f
; PORTB
subi pin, 8
rjmp _modpin
2:
cpi pin, 20         ; check for pin out of range
brcc _done
; PORTC
subi pin, 14
subi ZL, -3  ; PORTC = PORTB + 3

_modpin:
; bitno_to_bitval: calc 2^n, for n in 0..7
; 5 instr, 5 + 4n cycles, avg for Arduino UNO =~ 18 cycles
ldi bitval, 0x01
rjmp 2f
1:
lsl bitval
2:
subi pin, 1
brcc 1b

; now modify pin
ldd r0, Z+2         ; PORTx = PINx + 2
; eor r0, pinstate
cpse pinstate, r1
eor r0, bitval
and r0, bitval
st Z, r0            ; write to PINx
_done:
ret

; bitno_to_bitval: calc 2^n, for n in 0..7
; use lookup table = 7 cycles, 5 instr + 8B table
; align table to 8B to avoid adc ZH, r1
; X_ldiw 30, bitval_tbl
; add ZL, pin
; adc ZH, r1
; lpm bitval, Z

