; Ralph Doncaster 2020 MIT Licence open source
; pico crt: 12 bytes smaller than gcrt1 when used with -nostartfiles 

#include <avr/io.h>

	.macro	vector name
	.if (. - __vectors < _VECTORS_SIZE)
	.weak	\name
	.set	\name, __empty_interrupt
	rjmp	\name
	.endif
	.endm

	.section .vectors
	.global __vectors
__vectors:
	rjmp __init
	vector __vector_1
	vector __vector_2
	vector __vector_3
	vector __vector_4
	vector __vector_5
	vector __vector_6
	vector __vector_7
	vector __vector_8
	vector __vector_9
	vector __vector_10
	vector __vector_11
	vector __vector_12
	vector __vector_13
	vector __vector_14
	vector __vector_15
	vector __vector_16
	vector __vector_17
	vector __vector_18
	vector __vector_19

	.text
	.global __empty_interrupt
__empty_interrupt:
	reti

    .section .init0
__init:

    .section .init2
    ;clr r1
    ; clear all mem < 0xFF and regs < r28 instead of just bss
    .global __do_clear_bss
__do_clear_bss:
    clr ZH
    ldi ZL, 28
1:  st -Z, r31
    brne 1b


    .section .init9
; could put main init9 to save rjmp
    rjmp main
