#include "lcd.c"

const __flash char hello[] = "Hello, World!!!\nline 2 ";

void main(void) {
    lcd_init();
    
    lcd_puts(hello);
    
    lcd_putc('a');
    lcd_putc('b');
    lcd_putc('c');

	while(1);
}
