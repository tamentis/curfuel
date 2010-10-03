# 
# Skeleton for developing software for the Arduino Mega.  This
# Makefile seems to work well for my Arduino Mega connected to my
# Macbook.
#
# Requirements:
#
#   * on OSX you probably need to install the USB driver that comes
#     with the Arduino IDE
#   * make sure you have avr-gcc, avr-binutils, avr-libc and avrdude installed
#
#  -- Bjorn Borud <borud@borud.org>

### The name of your program and the main file
#
PROGRAM = curfuel

### Object files that are part of your program
#
OBJECTS = main.o wiring_analog.o wiring_digital.o wiring.o \
	pins_arduino.o wiring_shift.o \
	wiring_pulse.o WInterrupts.o \
	serialio.o

### The dype of device
#
DEVICE = atmega328p

### Clock speed of the device
#
CLOCK = 16000000

### Serial port we use for programming the Arduino
#
PORT = /dev/ttyACM0
# RATE = 57600
RATE = 115200
PROTO = stk500v1

######################################################################

AVRPREFIX=/usr/local/avr/bin
CC = $(AVRPREFIX)/avr-gcc
CPP = $(AVRPREFIX)/avr-g++
AVRDUDE = $(HOME)/projects/arduino/work/hardware/tools/avrdude
OBJCOPY = $(AVRPREFIX)/avr-objcopy
AVRSIZE = $(AVRPREFIX)/avr-size
# CFLAGS = -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -I.
CFLAGS = -Wall -g -w -ffunction-sections -fdata-sections -Os -DF_CPU=$(CLOCK) -DARDUINO=21 -mmcu=$(DEVICE) -I.
CPPFLAGS = -Wall -g -w -fno-exceptions -ffunction-sections -fdata-sections -Os -DF_CPU=$(CLOCK) -DARDUINO=21 -mmcu=$(DEVICE) -I.
RM = rm -f
FLASH = -U flash:w:$(PROGRAM).hex:i
AVRDUDE_FLAGS = -p$(DEVICE) -c$(PROTO) -P$(PORT) -b$(RATE) $(FLASH)

all:	$(PROGRAM).hex

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

.S.o:
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< $@

.c.s:
	$(CC) $(CFLAGS) -S $< -o $@

$(PROGRAM).elf: $(OBJECTS)
	$(CC) -Wl,--gc-sections $(CFLAGS) -o $(PROGRAM).elf $(OBJECTS) -lm

$(PROGRAM).hex: $(PROGRAM).elf
	$(RM) $(PROGRAM).hex
	$(OBJCOPY) -j .text -j .data -O ihex $(PROGRAM).elf $(PROGRAM).hex
	$(AVRSIZE) *.hex *.elf *.o

upload: $(PROGRAM).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS)

disasm: $(PROGRAM).elf
	$(AVRPREFIX)/avr-objdump -d $(PROGRAM).elf

clean:
	$(RM) $(OBJECTS) $(PROGRAM).elf $(PROGRAM).hex
