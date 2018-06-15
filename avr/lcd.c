// Nerd Ralph 2014, 2018 public domain software
// Control 2-line HD44780 LCD with 4 bit interface
// version 2.0 (2018) cleanup and optimizations

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
        Data1   8   Gnd      |
        Data2   9   2 (PD2)--*
        Data3   10  3 (PD3)
        Data4   11--4 (PD4)
        Data5   12--5 (PD5)
        Data6   13--6 (PD6)
        Data7   14--7 (PD7)
*/

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
#define RW_PIN (1<<PD1)
#endif

#define LCD_ISR_SAFE    0

/* instruction register commands, see HD44780U data sheet */
#define LCD_CLR               (1<<0)  /* DB0: clear display                  */
/* DB3: control lcd/cursor, DB2 display on/off */
#define LCD_ON                ((1<<3)|(1<<2))
#define LCD_CURSOR            (1<<1)  /*   DB1: turn cursor on               */
#define LCD_BLINK             (1<<0)  /*     DB0: blinking cursor ?          */
#define LCD_FUNC              (1<<5)  /* DB5: function set                   */
#define LCD_8BIT              (1<<4)  /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_2LINES            (1<<3)  /*   DB3: two lines (0->one line)      */
#define LCD_DDRAM             (1<<7)  /* DB7: set DD RAM address             */

const char init_seq[] = 
    { 0b00110011,               // set 8-bit mode 2x
      0b00110010,               // set 8-bit then 4-bit mode
      LCD_FUNC|LCD_2LINES,
//      LCD_FUNC,                 // 4-bit, 1-line mode
      LCD_ON };

#define sbi_lcd(pin) ((DATA_PORT) |= pin)
#define cbi_lcd(pin) ((DATA_PORT) &= ~pin)

void lcd_delay() {
    // delay 62us for a 50% margin over 37 + 4us typical timing
    _delay_us(62);
}

void lcd_long_delay() {
    // 2.3ms delay gives 50% margin over 1.52ms typical timing
    _delay_ms(2.3);
}    

// assumes EN_PIN is LOW in the beginning
// writes high nibble of data
void lcd_write_nibble( const uint8_t data ) {
    uint8_t tmp;
    
    sbi_lcd(EN_PIN);

    // need cli/sei if any ISR modifies DATA_PORT
    tmp = DATA_PORT & 0x0F;
    DATA_PORT = tmp | (data&0xF0);

    cbi_lcd(EN_PIN);
    lcd_delay();            // wait for instruction to complete
}

void lcd_write( const uint8_t data) {
    lcd_write_nibble(data);
    lcd_write_nibble(data<<4);
}

void lcd_command(const uint8_t command) {
    cbi_lcd(RS_PIN);
    lcd_write(command);
}

void lcd_data(const uint8_t data) {
    sbi_lcd(RS_PIN); 
    lcd_write(data);
}

void lcd_clear(){
    lcd_command(LCD_CLR);

    // this command takes longer than others
    lcd_long_delay();
}

void lcd_gotoxy(uint8_t x, uint8_t y){
    uint8_t addr = 0;
    if (y == 1) addr = 40;
    lcd_command(LCD_DDRAM + addr + x);
}

void lcd_newline(){
    lcd_command(LCD_DDRAM + 40); // move to start of 2nd line
}

void lcd_cursor_on(){
    lcd_command(LCD_ON|LCD_CURSOR|LCD_BLINK);
}

void lcd_cursor_off(){
    lcd_command(LCD_ON);
}

void lcd_init() {
    // default PORT state is low
    DATA_PORT_DIR = 0xF0 | RW_PIN | EN_PIN | RS_PIN;
    lcd_delay();                // wait in case EN low triggered command

    for (int i = 0; i < sizeof(init_seq); i++) {
        // RS is low, so calling lcd_write is safe
        lcd_write(init_seq[i]);
    }

/*
    lcd_write_nibble(LCD_FUNC|LCD_8BIT);
    //lcd_long_delay();
    lcd_delay();
    lcd_write_nibble(LCD_FUNC|LCD_8BIT);
    lcd_delay();
    lcd_write_nibble(LCD_FUNC|LCD_8BIT);
    lcd_delay();

    // set 4-bit mode
    lcd_write_nibble(LCD_FUNC);
    lcd_delay();

    // 2 lines, normal font, 4 bit
    lcd_command(LCD_FUNC|LCD_2LINES);


    // entry mode increment, don't shift is default state
    // lcd_command(LCD_ENTRY_MODE|LCD_INC);

    // display on, no cursor
    lcd_cursor_off();
*/

    lcd_clear();
}

void lcd_putc(char c) {
    if (c=='\n') {
        lcd_newline();
    } else {
        lcd_data(c);
    }
}

void lcd_puts(const __flash char * s) {
    char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }
}

