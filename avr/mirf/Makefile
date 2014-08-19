#DEVICE = attiny84
DEVICE = attiny88
#DEVICE = atmega328p
#CPU_SPEED = 16000000L
CPU_SPEED = 8000000
#DEVICE = atmega8
# TARGET = rxRf


# define the following, then include avr.mk
# DEVICE = atmega328p
# CPU_SPEED = 16000000
PROGRAMMER = usbasp -B 2
#  or PROGRAMMER = arduino -P com16

# or export DEVICE=atxxxx in shell 
# export TARGET=app

# CC = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
#CPP = avr-g++ --std=c++11 -mmcu=$(DEVICE)
CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED)
LDLIBS = the-shit.a
#CFLAGS += -fwhole-program -mrelax
#CFLAGS += -Os -Wall -flto -finline-limit=2
#CFLAGS += -Os -Wall -flto -mrelax -fno-inline
CFLAGS += -Os -Wall -flto -mrelax
#LDFLAGS +=
CFLAGS += -Wno-main

.PHONY: force

# all should be first rule?
all: $(LDLIBS) $(TARGET) $(TARGET).hex

compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(LIBS): compiler_flags

#the-shit.a: BBUart.o t32isr.o
#	avr-ar rcs $@ $^

the-shit.a: mirf.o spi.o
	avr-ar rcs $@ $^

sniff: sniff.o sniffASM.o
	$(CC) $^ -o $@

#$(TARGET).hex: $(TARGET)
#	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

# lfuse 0xe2 = 8Mhz internal RC, ckdiv1, 64ms SUT
fuses:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U lfuse:w:0xe2:m

clean:
	rm $(LIBS) $(TARGET) $(TARGET).hex *.o
