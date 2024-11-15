CC=avr-gcc
CXX=avr-g++
CFLAGS=-std=c99
CXXFLAGS=-std=c++11
MMCU=atmega328p
CPPFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MMCU) -DF_CPU=16000000UL
ifeq ($(OS),Windows_NT)	    
    PROGRAMMER_PORT=COM3
else
    PROGRAMMER_PORT=/dev/ttyUSB0
endif



TARGET=main
C_SOURCES=$(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c)
CPP_SOURCES=$(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp)
O_FILES=$(patsubst %.c,%.o,$(C_SOURCES))
O_FILES+=$(patsubst %.cpp,%.o,$(CPP_SOURCES))

object: $(TARGET).o

elf: $(TARGET).elf

hex: $(TARGET).hex

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.elf: $(O_FILES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

%.hex: %.elf
	avr-objcopy -O ihex $< $@

program: $(TARGET).hex
	avrdude -p $(MMCU) -c arduino -P $(PROGRAMMER_PORT) -U flash:w:$<

clean:
	rm -f $(TARGET).hex $(TARGET).elf *.o

.PHONY: program clean
