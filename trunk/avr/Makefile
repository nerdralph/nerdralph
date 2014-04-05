DEVICE = attiny85
# or export DEVICE=atxxxx in shell 
# export TARGET=app

CC = avr-gcc -mmcu=$(DEVICE)
#CFLAGS = -mmcu=$(DEVICE)
# LIBS = BBUart.o
CFLAGS += -Os -Wall
# CFLAGS += -L $(LIBS)

.PHONY: force
compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(LIBS): compiler_flags

all: $(LIBS) $(TARGET) $(TARGET).hex

$(TARGET).hex: $(TARGET)
	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c usbasp -U flash:w:$(TARGET).hex

clean:
	rm $(LIBS) $(TARGET) $(TARGET).hex
