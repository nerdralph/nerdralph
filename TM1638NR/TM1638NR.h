/* Ralph Doncaster 2018
 * TM1638NR.h - Library for addressing TM1638 I/O chips/card from Arduino.
 * MIT license.
 */

#ifndef TM1638NR_h
#define TM1638NR_h

class TM1638NR {
 public:
  // brightness levels 0-7
  static void reset(byte brightness = 2){
    sendCommand(ACTIVATE | (brightness & 0x07));
    for (uint8_t i = 16; i--; writeLoc(i, 0x00));
  }

  static void sendCommand(uint8_t cmd) {
    startTx(cmd);
    pinMode(STROBE, INPUT);
  }

  // 8 buttons from K3 supported
  static uint8_t readButtons();

  // bit 0 = LED 0 ... 
  static void setLEDs(uint8_t mask) {
    for ( uint8_t bit = 0; bit < 8; bit++) {
      // LEDs are at odd locations
      writeLoc(1 + (bit<<1), mask & 1);
      mask >>= 1;
    }
  }

  // write to seven segments
  static void displaySS(uint8_t position, uint8_t value) {
    writeLoc(position<<1, value);
  }

  static void displayHex(uint8_t position, uint8_t hex);

 private:
  static void clkLow()  __attribute__((always_inline)) {
    // ensure clock has risen to high
    while (digitalRead(CLOCK) == 0);
    pinMode(CLOCK, OUTPUT);
  }

  // open-drain output, data clocked on rising edge, LSB first
  // 1Mhz max
  static void send(uint8_t value) {
    uint8_t bits = 8;
    do {
      clkLow();
      if (value & 0x01)
        pinMode(DATA, INPUT);
      else
        pinMode(DATA, OUTPUT);
      value >>= 1;
      pinMode(CLOCK, INPUT);
      //delayMicroseconds(1);
    }
    while (--bits);
    pinMode(DATA, INPUT);       // release data line
  }

  // data clocked from slave on falling edge, LSB first
  // 1Mhz max
  static uint8_t receive() {
    uint8_t bits = 8;
    uint8_t value = 0;
    do {
      clkLow();
      value >>= 1;
      // delayMicroseconds(1);
      pinMode(CLOCK, INPUT);
      if (digitalRead(DATA)) value |= 0x80;
    }
    while (--bits);
    return value;
  }

  static void startTx(uint8_t value) {
    pinMode(STROBE, OUTPUT);
    send(value);
  }

  static void writeLoc(uint8_t position, uint8_t value) {
    startTx(0xC0 + position);
    startTx(value);
    pinMode(STROBE, INPUT);
  }

  static const byte STROBE;
  static const byte CLOCK;
  static const byte DATA;

  enum COMMAND {
    ACTIVATE = 0x88,
    BUTTONS = 0x42,
    WRITE_LOC = 0x44
  };

};

inline uint8_t TM1638NR::readButtons() {
  uint8_t buttons = 0;
  startTx(BUTTONS);
  delayMicroseconds(1);           // tWAIT

  for (uint8_t i = 0; i < 4; i++) {
    uint8_t bits = receive() << i;
    buttons |= bits;
  }

  pinMode(STROBE, INPUT);
  return buttons;
}

/*
segments:
  -0-
 5   1
  -6-
 4   2
  -3- .7
*/

static const int8_t HEXSS[] PROGMEM = {
  0x3F, /* 0 */
  0x06, /* 1 */
  0x5B, /* 2 */
  0x4F, /* 3 */
  0x66, /* 4 */
  0x6D, /* 5 */
  0x7D, /* 6 */
  0x07, /* 7 */
  0x7F, /* 8 */
  0x6F, /* 9 */
  0x77, /* A */
  0x7C, /* B */
  0x39, /* C */
  0x5E, /* D */
  0x79, /* E */
  0x71, /* F */
};

void TM1638NR::displayHex(uint8_t position, uint8_t hex) {
  displaySS(position, pgm_read_byte(HEXSS + (hex & 0x0F)));
}

#endif
