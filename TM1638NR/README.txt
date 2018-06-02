# TM1637NR

A minimal Arduino-compatible library for TM1638 "LED&KEY" modules.
Supports 8 buttons, 8 LEDs, and an 8-digit 7-segment display.

To use, copy to your Arduino libraries directory, include TM1638NR.h in
your sketch, and define the 3 pins:

const byte TM1638NR::STROBE = 0;
const byte TM1638NR::CLOCK = 1;
const byte TM1638NR::DATA = 2;

