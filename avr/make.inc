# Ralph Doncaster 2020
# master makefile for avr projects
# create subdir for project, containing another subdir src

DEVICE ?= attiny85
CPU_SPEED ?= 8000000
TARGET := $(notdir $(shell pwd))

PROGRAMMER = usbasp
#PROGRAMMER = arduino -P com2 -b 57600

CXX = avr-gcc --std=gnu++11
CC = avr-gcc --std=gnu11
CPPFLAGS += -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED)
CPPFLAGS += -Os -mrelax -Wall -Wno-main
CPPFLAGS += -ffunction-sections -Wl,--gc-sections
CPPFLAGS += -flto -Wl,-flto

# use := for evaluation from the including makefile
SOURCES := src/*.[cS]

.PHONY: $(TARGET)
$(TARGET): $(SOURCES)
	$(CC) $(CPPFLAGS) $^ -o $@

flash:  $(TARGET)
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET):e

reset:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER)

eedump:
	avrdude -C /etc/avrdude.conf -p $(DEVICE) -c $(PROGRAMMER) -U eeprom:r:eedump.bin:r

debug:
	@echo CPPFLAGS = $(CPPFLAGS)
