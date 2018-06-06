// shift1 (c) 2018 Ralph Doncaster, MIT licence
// use with a ~33K resistor and diode from SRCLK to SER
// see http://nerdralph.blogspot.com/2018/06/fast-1-wire-shift-register-control.html

const uint8_t SHFT = 0;

void shiftOne(uint8_t pin, uint8_t value) {
  const uint8_t pinMask = 1<<pin;
  uint8_t i = 8;
  do
  {
    PORTB |= 1<<pin;
    if (value & 0x80) PINB = pinMask;
    PINB = pinMask;
    asm("sbrs %0,7\nlpm":: "r"(value):"r0");
    value <<= 1;
    i--;
  }
  while(i);
  asm("nop");
  PORTB |= 1<<pin;   // leave pin high
}

void setup() {
  pinMode(SHFT,OUTPUT);
  digitalWrite(SHFT, HIGH);
}

void loop() {
  static uint8_t level = 0;
  for (uint8_t i = 255; i--;){
    if (i >= level) shiftOne(SHFT, 0xFF);
    else shiftOne(SHFT, 0x00);
    delayMicroseconds(10);
  }
  level++;
}

