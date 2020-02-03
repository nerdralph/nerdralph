Ralph Doncaster 2020 open source MIT license

picoUART is a pico-size simplex high-speed AVR bit-bang UART.
Code size is less than 80 bytes total for transmit and receive.
picoUART timing is accurate to the cycle (+- 0.5 cycle error);
this is less than 1% at 8Mhz.

picoUART is also capable of single-pin (shared Rx/Tx) operation.

picoUART is a header-only library that is compatible with the Arduino API
but is not dependent on any Arduino functions so it can be used with
avr-libc.

