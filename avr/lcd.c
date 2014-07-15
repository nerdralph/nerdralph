// Nerd Ralph 2014 public domain software
// Control 2-line HD44780 LCD with 4 bit interface
// based on code:
// Copyright (C) 2012 Joonas Pihlajamaa. Released to public domain.
// also based on code by Peter Fleury
// http://homepage.hispeed.ch/peterfleury/avr-software.html#libs

/*
Suggested wiring between LCD module and Arduino Pro Mini
http://nerdralph.blogspot.ca/2014/07/whats-up-with-hd44780-lcd-displays.html

     VSS (Gnd)  1----Gnd
           VDD  2----VSS
 VE (contrast)  3
Register Select 4------------*
    Read/Write  5--Gnd       |
        Enable  6---Rx (PD0) |
        Data0   7   Rst      |
        Data0   8   Gnd      |
        Data0   9   2 (PD2)--*
        Data0   10  3 (PD3)
        Data0   11--4 (PD4)
        Data0   12--5 (PD5)
        Data0   13--6 (PD6)
        Data0   14--7 (PD7)
*/
#define byte unsigned char

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#ifndef DATA_PORT
#warning Using default of PORTD for LCD I/O
#define DATA_PORT PORTD
#define DATA_PORT_DIR DDRD

#define RS_PIN (1<<PD2)
#define EN_PIN (1<<PD0)
#endif

#define LCD_ISR_SAFE    0

/* instruction register commands, see HD44780U data sheet */
#define LCD_CLR               (1<<0)  /* DB0: clear display                  */
#define LCD_HOME              (1<<1)  /* DB1: return to home position        */
#define LCD_ENTRY_MODE        (1<<2)  /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         (1<<1)  /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       (1<<0)  /*   DB2: 1=display shift on           */
#define LCD_ON                (1<<3)  /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        (1<<2)  /*   DB2: turn display on              */
#define LCD_ON_CURSOR         (1<<1)  /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          (1<<0)  /*     DB0: blinking cursor ?          */
#define LCD_MOVE              (1<<4)  /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         (1<<3)  /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        (1<<2)  /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          (1<<5)  /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     (1<<4)  /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   (1<<3)  /*   DB3: two lines (0->one line)      */
#define LCD_DDRAM             (1<<7)  /* DB7: set DD RAM address             */

#define SET_CTRL_BIT(pin) ((DATA_PORT) |= pin)
#define CLEAR_CTRL_BIT(pin) ((DATA_PORT) &= ~pin)

void lcd_delay() {
    // delay 65us for comands
    _delay_us(65);
}    

void lcd_long_delay() {
    // delay = 2600us;
    _delay_us(2600);
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
#ifdef LCD_ISR_SAFE
    cli();
#endif
    DATA_PORT |= (0xF0 | RS_PIN);    // high while idle
#ifdef LCD_ISR_SAFE
    sei();
#endif
    lcd_delay();          // wait for instruction to complete
}

void lcd_command(const byte command) {
    lcd_write(command, 0);
}

void lcd_clear(){
    lcd_command(LCD_CLR);

    // this command takes longer than others
    lcd_long_delay();
}

void lcd_gotoxy(byte x, byte y){
    byte addr = 0;
    if (y == 1) addr = 40;
    lcd_command(LCD_DDRAM + addr + x);
}

void lcd_newline(){
    lcd_command(LCD_DDRAM + 40); // move to start of 2nd line
}

void lcd_cursor_on(){
    lcd_command(LCD_ON|LCD_ON_DISPLAY|LCD_ON_CURSOR|LCD_ON_BLINK);
}

void lcd_cursor_off(){
    lcd_command(LCD_ON|LCD_ON_DISPLAY);
}

inline void lcd_init() {
    // tie low for write mode
    CLEAR_CTRL_BIT(RS_PIN);
    CLEAR_CTRL_BIT(EN_PIN);

    // DDR address is one less than port
    DATA_PORT_DIR = 0xF0 | EN_PIN | RS_PIN;

    // if sut fuses = 4ms or 64ms(default), no need to wait before init
    // lcd_long_delay();

    lcd_write_nibble(LCD_FUNCTION|LCD_FUNCTION_8BIT);
    lcd_delay();

    lcd_write_nibble(LCD_FUNCTION|LCD_FUNCTION_8BIT);
    lcd_delay();

    // set 4-bit mode
    lcd_write_nibble(LCD_FUNCTION);
    lcd_delay();

    // 2 lines, normal font, 4 bit
    lcd_command(LCD_FUNCTION|LCD_FUNCTION_2LINES);

    lcd_clear();

    // entry mode increment, don't shift
    lcd_command(LCD_ENTRY_MODE|LCD_ENTRY_INC);

    // display on, no cursor
    lcd_cursor_off();
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

