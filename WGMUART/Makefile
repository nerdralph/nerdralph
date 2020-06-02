MCU     ?= attiny85 
SPEED   ?= 8000000L 
CC      = avr-gcc
OPTIONS = -Os -flto -mmcu=$(MCU) -DF_CPU=$(SPEED) -ffixed-r2
TARGET  = wgmuart

.PHONY: $(TARGET)

$(TARGET): *.[cS]
	$(CC) $(OPTIONS) $^ -o $@
	avr-size $@

PROGRAMMER ?= usbasp -B 4
flash:  $(TARGET)
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET):e

