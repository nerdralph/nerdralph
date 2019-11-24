#DEVICE = attiny85
#DEVICE = attiny84
#DEVICE = attiny88
#DEVICE = atmega8a
#DEVICE = atmega328p
DEVICE ?= attiny13
CPU_SPEED = 9500000
#CPU_SPEED = 1150000
CPU_SPEED ?= 8000000
# or export DEVICE=atxxxx in shell 
# export TARGET=app

# bit-bang uart definitions
SERIAL_BAUD = 115200
SERIAL_BIT_TIME = $(CPU_SPEED)/$(SERIAL_BAUD)

PROGRAMMER = usbasp -B 4
#PROGRAMMER = usbasp
#PROGRAMMER = arduino -P com16

# CC = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
CPP = avr-g++ --std=c++11 -mmcu=$(DEVICE)
#CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED) -DBIT_TIME=$(SERIAL_BIT_TIME)
CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED)
LDLIBS = libs/libs.a
#CFLAGS += -fwhole-program
CFLAGS += -ffunction-sections -Wl,--gc-sections
CFLAGS += -Os -Wall -mrelax
CFLAGS += -flto
#CFLAGS += -nodefaultlibs
#CFLAGS += -Wl,--defsym,exit=0
CFLAGS += -Ilibs/
#LDFLAGS +=
CFLAGS += -Wno-main
CFLAGS += -ffreestanding    # int main() not required
CFLAGS += --std=gnu99

LIB_CSRC = $(shell ls libs/*/*.c)
LIB_ASMSRC = $(shell ls libs/*/*.S)
LIBOBJ = $(LIB_CSRC:.c=.o) $(LIB_ASMSRC:.S=.o)

.PHONY: force

all: $(LDLIBS) $(TARGET) $(TARGET).hex

compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

# $(LDLIBS): compiler_flags

#BBUart.a: BBUart.o
#	avr-ar r BBUart.a BBUart.o

libs/libs.a: $(LIBOBJ)
	avr-ar rcs $@ $^

sniff: sniff.o sniffASM.o
	$(CC) $^ -o $@

$(TARGET).hex: $(TARGET)
	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex -F

eedump:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U eeprom:r:eedump.hex:i

# lfuse 0xe2 = 8Mhz internal RC, ckdiv1, 64ms SUT
fuses:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U lfuse:w:0xe2:m

# DWEN = bit 3
t13dwen:
	avrdude -C /etc/avrdude.conf -p t13 -c $(PROGRAMMER) -U hfuse:w:0xf7:m

clean:
	rm -f $(LDLIBS) $(TARGET) $(TARGET).hex *.o

debug:
	echo CFLAGS = $(CFLAGS)
