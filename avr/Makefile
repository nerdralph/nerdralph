DEVICE = attiny85
# or export DEVICE=atxxxx in shell 
# export TARGET=app

CC = avr-gcc -mmcu=$(DEVICE)
#CFLAGS = -mmcu=$(DEVICE)
CFLAGS += -Os -Wall

all: $(TARGET) $(TARGET).hex

$(TARGET).hex: $(TARGET)
	avr-objcopy -j .text -j .data -O ihex $< $@

%.hex: %
	avr-objcopy -j .text -j .data -O ihex $< $@

flash:  $(TARGET).hex
	../avrdude-6.0.1/avrdude.exe -p $(DEVICE) -c usbasp -U flash:w:$(TARGET).hex
