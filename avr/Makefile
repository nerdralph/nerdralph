#DEVICE = attiny85
#DEVICE = attiny84
#DEVICE = attiny88
#DEVICE = atmega8a
#DEVICE = atmega328p
DEVICE ?= attiny13
CPU_SPEED ?= 9600000
# or export DEVICE=atxxxx in shell 
# export TARGET=app

PROGRAMMER = usbasp -B 3
#PROGRAMMER = arduino -P com37

CXX = avr-gcc --std=gnu++11
CC = avr-gcc --std=gnu11
CPPFLAGS += -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED)
CPPFLAGS += -Os -mrelax -Woverflow
CPPFLAGS += -ffunction-sections -Wl,--gc-sections
CPPFLAGS += -flto -Wl,-flto
# CPPFLAGS += -ffreestanding    # int main() not required
#LDLIBS = libs/libs.a
#CFLAGS += -nodefaultlibs
#CFLAGS += -Wl,--defsym,exit=0
#CFLAGS += -Ilibs/

#LIB_CSRC = $(shell ls libs/*/*.c)
#LIB_ASMSRC = $(shell ls libs/*/*.S)
#LIBOBJ = $(LIB_CSRC:.c=.o) $(LIB_ASMSRC:.S=.o)

.PHONY: force

all: $(LDLIBS) $(TARGET) $(TARGET).hex

compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(LDLIBS): compiler_flags

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
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

eedump:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U eeprom:r:eedump.bin:r

# lfuse 0xe2 = 8Mhz internal RC, ckdiv1, 64ms SUT
fuses:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U lfuse:w:0xe2:m

# t13 @ 4.8Mhz
t1348:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U lfuse:w:0x39:m

# DWEN = bit 3
t13dwen:
	avrdude -C /etc/avrdude.conf -p t13 -c $(PROGRAMMER) -U hfuse:w:0xf7:m

clean:
	rm -f $(LDLIBS) $(TARGET) $(TARGET).hex *.o

debug:
	echo CFLAGS = $(CFLAGS)
