// lcd.h

#define DATA_PORT_DIR DDRD
#define DATA_PORT PORTD

#define RW_PIN (1<<PD1)

#define RS_PIN (1<<PD2)
#define EN_PIN (1<<PD0)

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
