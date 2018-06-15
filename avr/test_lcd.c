#include "lcd.c"

#define F(s)    ((const __flash char *)(s))

const __flash char hello[] = "Hello, World!!!\nbuild "__TIME__;
//const __flash char hello[] = "Hello, World!!!\n22:46 line 2";

void main(void) {
    lcd_init();
    char c = 0;
	while (1) {
        lcd_clear();
        lcd_puts(hello);
        lcd_putc(' ');
        lcd_putc(c++);
        _delay_ms(200);
    }

    // F() macro needs debugging
    //lcd_puts(F("Hi"));
}
