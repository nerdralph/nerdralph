# Ralph Doncaster 2020
# master makefile for avr projects
# TARGET is always rebuilt, obviating the need for dependency generation
# create subdir for project, containing another subdir src

DEVICE ?= attiny85
CPU_SPEED ?= 8000000
TARGET := $(notdir $(shell pwd)).elf

PROGRAMMER = avrdude -p $(DEVICE) -c usbasp
#PROGRAMMER = avrdude -p $(DEVICE) -c arduino -P com2 -b 57600

CXX = avr-gcc
CC = avr-gcc
CPPFLAGS += -mmcu=$(DEVICE) -DF_CPU=$(CPU_SPEED)
CPPFLAGS += -Os -mrelax -Wall
CPPFLAGS += -ffunction-sections -Wl,--gc-sections
CPPFLAGS += -flto -Wl,-flto

# use := for evaluation from the including makefile
SOURCES := $(wildcard src/*.c* src/*.S)

.PHONY: $(TARGET)
$(TARGET): $(SOURCES)
	$(CC) $(CPPFLAGS) $^ -o $@

flash:  $(TARGET)
	$(PROGRAMMER) -U flash:w:$(TARGET):e

reset:
	$(PROGRAMMER)

eedump:
	$(PROGRAMMER) -U eeprom:r:eedump.bin:r

debug:
	@echo CPPFLAGS = $(CPPFLAGS)
	@echo SOURCES = $(SOURCES)
