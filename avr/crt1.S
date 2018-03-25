/* Copyright (c) 2002, Marek Michalkiewicz <marekm@amelek.gda.pl>
   Copyright (c) 2007, 2008 Eric B. Weddington
   Copyright (c) 2018 Ralph Doncaster http://github.com/nerdralph
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

// taken from gcrt1.S and optimized by Ralph Doncaster 2018 

#include <avr/io.h>

// some macros copied from "macros.inc"

#if !defined(__tmp_reg__)
    #if defined(__AVR_TINY__)
        #define __tmp_reg__ r16
    #else
        #define __tmp_reg__ r0
    #endif
#endif

#if !defined(__zero_reg__)
    #if defined(__AVR_TINY__)
        #define __zero_reg__ r17
    #else
        #define __zero_reg__ r1
    #endif
#endif

#if (__AVR_HAVE_JMP_CALL__)
  #define XJMP jmp
  #define XCALL call
#else
  #define XJMP rjmp
  #define XCALL rcall
#endif


	.macro	vector name
	.if (. - __vectors < _VECTORS_SIZE)
	.weak	\name
	sleep
	.endif
	.endm

	.section .vectors,"ax",@progbits
	.global	__vectors
	.func	__vectors
__vectors:
	XJMP	__init
	vector	__vector_1
	vector	__vector_2
	vector	__vector_3
	vector	__vector_4
	vector	__vector_5
	vector	__vector_6
	vector	__vector_7
	vector	__vector_8
	vector	__vector_9
	vector	__vector_10
	vector	__vector_11
	vector	__vector_12
	vector	__vector_13
	vector	__vector_14
	vector	__vector_15
	vector	__vector_16
	vector	__vector_17
	vector	__vector_18
	vector	__vector_19
	vector	__vector_20
	vector	__vector_21
	vector	__vector_22
	vector	__vector_23
	vector	__vector_24
	vector	__vector_25
	vector	__vector_26
	vector	__vector_27
	vector	__vector_28
	vector	__vector_29
	vector	__vector_30
	vector	__vector_31
	vector	__vector_32
	vector	__vector_33
	vector	__vector_34
	vector	__vector_35
	vector	__vector_36
	vector	__vector_37
	vector	__vector_38
	vector	__vector_39
	vector	__vector_40
	vector	__vector_41
	vector	__vector_42
	.endfunc


	.section .init0,"ax",@progbits
	.weak	__init
;	.func	__init
__init:

#ifndef __AVR_ASM_ONLY__
	.weak	__stack

	/* By default, malloc() uses the current value of the stack pointer
	   minus __malloc_margin as the highest available address.

	   In some applications with external SRAM, the stack can be below
	   the data section (in the internal SRAM - faster), and __heap_end
	   should be set to the highest address available for malloc().  */
	.weak	__heap_end
	.set	__heap_end, 0

	.section .init2,"ax",@progbits
	clr	__zero_reg__

#ifdef __AVR_HAVE_RAMPD__
	out	AVR_RAMPD_ADDR, __zero_reg__
	out	AVR_RAMPX_ADDR, __zero_reg__
	out	AVR_RAMPY_ADDR, __zero_reg__
	out	AVR_RAMPZ_ADDR, __zero_reg__
#endif

#if defined(__GNUC__) && ((__GNUC__ <= 3) || (__GNUC__ == 4 && __GNUC_MINOR__ <= 3))
#if BIG_CODE
	/* Only for >64K devices with RAMPZ, replaces the default code
	   provided by libgcc.S which is only linked in if necessary.  */

	.section .init4,"ax",@progbits
	.global __do_copy_data
__do_copy_data:
	ldi	r17, hi8(__data_end)
	ldi	r26, lo8(__data_start)
	ldi	r27, hi8(__data_start)
	ldi	r30, lo8(__data_load_start)
	ldi	r31, hi8(__data_load_start)

	/* On the enhanced core, "elpm" with post-increment updates RAMPZ
	   automatically.  Otherwise we have to handle it ourselves.  */

#ifdef __AVR_ENHANCED__
	ldi	r16, hh8(__data_load_start)
#else
	ldi	r16, hh8(__data_load_start - 0x10000)
.L__do_copy_data_carry:
	inc	r16
#endif
	out	AVR_RAMPZ_ADDR, r16
	rjmp	.L__do_copy_data_start
.L__do_copy_data_loop:
#ifdef __AVR_ENHANCED__
	elpm	r0, Z+
#else
	elpm
#endif
	st	X+, r0
#ifndef __AVR_ENHANCED__
	adiw	r30, 1
	brcs	.L__do_copy_data_carry
#endif
.L__do_copy_data_start:
	cpi	r26, lo8(__data_end)
	cpc	r27, r17
	brne	.L__do_copy_data_loop
#ifdef __AVR_HAVE_RAMPD__
	out	AVR_RAMPZ_ADDR, __zero_reg__
#endif /* __AVR_HAVE_RAMPD__*/

#endif /* BIG_CODE */
#endif /* defined(__GNUC__) && ((__GNUC__ <= 3) || (__GNUC__ == 4 && __GNUC_MINOR__ <= 3)) */

	.set	__stack, RAMEND
#endif /* !__AVR_ASM_ONLY__ */

	.section .init9,"ax",@progbits
	XJMP	main
;	.endfunc

    .section .note.gnu.avr.deviceinfo, "", @note
#define NOTE_NAME "AVR"
#ifdef __AVR_DEVICE_NAME__
    #define DEVICE_NAME STR(__AVR_DEVICE_NAME__)
#else
    #define DEVICE_NAME ""
#endif

    .long .L__note_name_end - .L__note_name_start
    .long .L__desc_end - .L__desc_start
    .long 1 ; Type 1 - this is the only known note type for AVR.
.L__note_name_start:
    .asciz NOTE_NAME
    .balign 4
.L__note_name_end:
.L__desc_start:
#ifdef FLASHSTART
    .long FLASHSTART
#else
    .long 0
#endif
#ifdef PROGMEM_SIZE
    .long PROGMEM_SIZE
#elif FLASHEND > 0
    .long FLASHEND + 1
#else
    .long FLASHEND
#endif
    .long RAMSTART
#ifdef RAMSIZE
    .long RAMSIZE
#elif RAMEND > 0
    .long RAMEND - RAMSTART + 1
#else
    .long RAMEND
#endif
#ifdef E2START
    .long E2START
#else
    .long 0
#endif
#ifdef EEPROM_SIZE
    .long EEPROM_SIZE
#elif E2END > 0
    .long E2END + 1
#else
    .long E2END
#endif
    /* String offsets table.
    Index 0 - Size of offset table in bytes
    Index 1 - Device name byte offset
    */
.L__stroffsettab_start:
.long .L__stroffsettab_end - .L__stroffsettab_start /* Size of index table in bytes */
.long .L__device_name_start - .L__strtab_start /* Byte offset of device name */
.L__stroffsettab_end:
    /* String table for storing arbitrary strings.
    Offsets are stored in the string offset table above */
.L__strtab_start:
    .byte 0
.L__device_name_start:
    .asciz DEVICE_NAME
.L__device_name_end:
    .byte 0
.L__strtab_end:
.L__desc_end:
    .balign 4

  /* Set REGION_LENGTH symbol values for well known memory regions.
  The default linker script uses these symbols to set MEMORY 
  region lengths, and by defining these here, the linker can detect
  memory overflows accurately on a per device basis, since the 
  values are picked up from the device header file.
  */

  .weak __FUSE_REGION_LENGTH__
  .set __FUSE_REGION_LENGTH__, FUSE_MEMORY_SIZE
