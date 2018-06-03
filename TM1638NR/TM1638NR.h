/* Ralph Doncaster 2018
 * TM1638NR.h - Library for addressing TM1638 I/O chips/card from Arduino.
 * MIT license.
 */

#ifndef TM1638NR_h
#define TM1638NR_h

class TM1638NR {
 public:
  // brightness levels 0-7
  static void reset(byte brightness = 3);

  static void sendCommand(uint8_t cmd) {
    startTx(cmd);
    pinMode(STROBE, INPUT);
  }

  // 8 buttons from K3 supported
  static uint8_t readButtons();

  static void setLEDs(uint8_t mask);

  // write to seven segments
  static void displaySS(uint8_t position, uint8_t value) {
    writeLoc(position<<1, value);
  }

  static void displayHex(uint8_t position, uint8_t hex);

 private:
  static void startTx(uint8_t value) {
    pinMode(STROBE, OUTPUT);
    shiftOut(DATA, CLOCK, LSBFIRST, value);
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
    ACTIVATE = 0x8A,
    BUTTONS = 0x42,
    WRITE_LOC = 0x44
  };
};

inline void TM1638NR::reset(byte brightness) {
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  sendCommand(ACTIVATE | (brightness & 0x07));
  for (uint8_t i = 16; i--; writeLoc(i, 0x00));
}

inline uint8_t TM1638NR::readButtons() {
  uint8_t buttons = 0;
  startTx(BUTTONS);
  pinMode(DATA, INPUT);

  for (uint8_t i = 0; i < 4; i++) {
    uint8_t bits = shiftIn(DATA, CLOCK, LSBFIRST) << i;
    buttons |= bits;
  }

  pinMode(DATA, OUTPUT);
  pinMode(STROBE, INPUT);
  return buttons;
}

// bit 0 = LED 0 ... 
inline void TM1638NR::setLEDs(uint8_t mask) {
  for ( uint8_t bit = 0; bit < 8; bit++){
    // LEDs are at odd locations
    writeLoc(1 + (bit<<1), mask & 1);
    mask >>= 1;
  }
}

/*
segments:
  -0-
 5   1
  -6-
 4   2
  -3- .7
*/

const uint8_t hexss[] PROGMEM = {
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

inline void TM1638NR::displayHex(uint8_t position, uint8_t hex) {
  displaySS(position, pgm_read_byte(hexss + (hex & 0x0F)));
}

#endif
