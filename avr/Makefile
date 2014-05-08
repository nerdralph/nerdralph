#DEVICE = attiny85
DEVICE = atmega328p
DEVICE = atmega8
# or export DEVICE=atxxxx in shell 
# export TARGET=app

PROGRAMMER = usbasp
#PROGRAMMER = arduino -P com15

# CC = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=16000000
LDLIBS = BBUart.a
#CFLAGS += -Os -Wall
#CFLAGS += -fwhole-program -mrelax
CFLAGS += -Os -Wall -flto
#LDFLAGS +=
CFLAGS += -Wno-main

.PHONY: force

all: $(LDLIBS) $(TARGET) $(TARGET).hex

compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(LIBS): compiler_flags

BBUart.a: BBUart.o
	avr-ar r BBUart.a BBUart.o

#$(TARGET).hex: $(TARGET)
#	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

clean:
	rm $(LIBS) $(TARGET) $(TARGET).hex *.o
