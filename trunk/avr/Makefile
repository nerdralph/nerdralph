DEVICE = attiny85
# or export DEVICE=atxxxx in shell 
# export TARGET=app

# CC = avr-g++ --std=c++11 -mmcu=$(DEVICE) -DF_CPU=8000000
CC = avr-gcc -mmcu=$(DEVICE) -DF_CPU=8000000
LDLIBS = BBUart.o
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

#$(TARGET).hex: $(TARGET)
#	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c usbasp -U flash:w:$(TARGET).hex

clean:
	rm $(LIBS) $(TARGET) $(TARGET).hex
