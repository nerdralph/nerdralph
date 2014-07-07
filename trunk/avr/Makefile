#DEVICE = attiny85
DEVICE = attiny84
#DEVICE = attiny88
DEVICE = atmega328p
#DEVICE = atmega8
# or export DEVICE=atxxxx in shell 
# export TARGET=app

#PROGRAMMER = usbasp -B 4
PROGRAMMER = arduino -P com16

# CC = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
CPP = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=8000000
LDLIBS = the-shit.a
#CFLAGS += -fwhole-program -mrelax
#CFLAGS += -Os -Wall -flto -finline-limit=2
CFLAGS += -Os -Wall -flto -mrelax
#CFLAGS += -Os -Wall -mrelax
#LDFLAGS +=
CFLAGS += -Wno-main

.PHONY: force

all: $(LDLIBS) $(TARGET) $(TARGET).hex

compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(LIBS): compiler_flags

#BBUart.a: BBUart.o
#	avr-ar r BBUart.a BBUart.o

the-shit.a: BBUart.o t32isr.o
	avr-ar rcs $@ $^

sniff: sniff.o sniffASM.o
	$(CC) $^ -o $@

#$(TARGET).hex: $(TARGET)
#	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

clean:
	rm $(LIBS) $(TARGET) $(TARGET).hex *.o
