// Nerd Ralph 2014 public domain software
// Control 16 character LCD (2x8 chars) with 4 bit interface
// based on code:
// Copyright (C) 2012 Joonas Pihlajamaa. Released to public domain.
// also based on code by Peter Fleury
// http://homepage.hispeed.ch/peterfleury/avr-software.html#libs

#define byte unsigned char

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#include "lcd.h"

#define SET_CTRL_BIT(pin) ((DATA_PORT) |= pin)
#define CLEAR_CTRL_BIT(pin) ((DATA_PORT) &= ~pin)

void delay65us() {
    _delay_us(2600);
}    

void delay1ms() {
    _delay_ms(1);
}

// assumes EN_PIN is LOW in the beginning
// writes high nibble of data
void lcd_write_nibble( const byte data ) {
    byte tmp;
    
    SET_CTRL_BIT(EN_PIN);

#ifdef LCD_ISR_SAFE
    cli();
#endif
    tmp = DATA_PORT & 0x0F;
    DATA_PORT = tmp | (data&0xF0);
#ifdef LCD_ISR_SAFE
    sei();
#endif

    CLEAR_CTRL_BIT(EN_PIN);
}

void lcd_write( const byte data, const char rs) {
    if(rs == 0)
        CLEAR_CTRL_BIT(RS_PIN); 

    lcd_write_nibble(data);
    lcd_write_nibble(data << 4);
    DATA_PORT |= (0xF0 | RS_PIN);    // high while idle
    delay65us();          // wait for instruction to complete
}

void lcd_command(const byte command) {
    lcd_write(command, 0);
}

void lcd_clear(){
    lcd_command(LCD_CLR);
}

void lcd_newline(){
    lcd_command(0x80 + 40); // move to 2nd line
}

inline void lcd_init() {
    // tie low for write mode
    CLEAR_CTRL_BIT(RW_PIN);
    CLEAR_CTRL_BIT(RS_PIN);
    CLEAR_CTRL_BIT(EN_PIN);

    DATA_PORT_DIR = 0xF0 | EN_PIN | RS_PIN | RW_PIN; 

    // if sut fuses = 4ms or 64ms(default), no need to wait before init

    lcd_write_nibble(LCD_FUNCTION|LCD_FUNCTION_8BIT);
    delay65us();

    lcd_write_nibble(LCD_FUNCTION|LCD_FUNCTION_8BIT);
    delay65us();

    // set 4-bit mode
    lcd_write_nibble(LCD_FUNCTION);
    delay65us();

    // 2 lines, normal font, 4 bit
    lcd_command(LCD_FUNCTION|LCD_FUNCTION_2LINES);

    lcd_clear();

    // entry mode increment, don't shift
    lcd_command(LCD_ENTRY_MODE|LCD_ENTRY_INC);
    //lcd_command(0x80); // DDRAM address 0 

    // display on, no cursor
    lcd_command(LCD_ON|LCD_ON_DISPLAY);
}

void lcd_putc(char c) {
    if (c=='\n') {
        lcd_newline();
    } else {
        lcd_write(c, 1);
    }
}

void lcd_puts(const __flash char * s) {
    char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }
}

void delay1s() {_delay_ms(1000);}

const __flash char hello[] = "Hello, World!!!\nline 2";
const __flash char msg2[] = " count:";

int main(void) {
    unsigned char i = 0;
    unsigned char hi;
    char lo;
    unsigned int ascii;
    char message[2];
    
    lcd_init();
    
    lcd_puts(hello);
    
    delay1s();
        
    lcd_puts(msg2);

	while(1);
	while(1) {
        if(++i > 99)
            i = 1;

/*
        if(i >= 10)
            message[0] = i/10+'0';
        else
            message[0] = ' ';
        message[1] = i%10+'0';

        ascii = byte_to_a(i);

        message[0] = * (( (char*) &ascii ) +1);
        message[1] = (char)ascii;

*/
        hi = '0'-1;
        lo = i;
        do {
            hi++;
            lo -=10;
        } while ( lo >= 0 );

        message[0] = hi;
        message[1] = lo + '0' + 10;

        lcd_puts(message);
        delay1s();
    } 

	return 1;
}
