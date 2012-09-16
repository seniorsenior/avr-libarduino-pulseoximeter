# AVR Makefile

PROG=avr-arduino
CPU=atmega328p
#CPU=atmega168

CFLAGS= -g -Os -Wall -Wstrict-prototypes -Wa,-ahlms=$(PROG).lst -mmcu=$(CPU) -DF_CPU=16000000

LFLAGS= -Wl,-Map=$(PROG).map,--cref -mmcu=$(CPU) -lm
# use LFLAGS below if you need to printf floating point numbers
#LFLAGS= -Wl,-u,vfprintf,-Map=$(PROG).map,--cref -mmcu=$(CPU) -lprintf_min -lm
INCL = 
SRC = main.c libarduino.c tsl230.c
OBJ = $(SRC:.c=.o)

# default target when "make" is run w/o arguments
all: $(PROG).rom

# compile serialecho.c into serialecho.o
%.o: %.c
	avr-gcc -c $(CFLAGS) -I. $*.c
	
# link up sample.o and timer.o into sample.elf
$(PROG).elf: $(OBJ)
	avr-gcc $(OBJ) $(LFLAGS) -o $(PROG).elf

$(OBJ): $(INCL)

# copy ROM (FLASH) object out of sample.elf into sample.rom
$(PROG).rom: $(PROG).elf
	avr-objcopy -O srec $(PROG).elf $(PROG).rom
	./checksize $(PROG).elf

# command to program chip (optional) (invoked by running "make install")
install:
	# uncomment the line below to use the SparkFun pocket ISP programmer
	# avrdude -p atmega168 -c usbtiny -U flash:w:$(PROG).rom 
	# uncomment the line below for atmega328 based Deimul*
	 avrdude -p m328p -P /dev/ttyUSB0 -c stk500v1 -b 57600 -F -u -U flash:w:$(PROG).rom
	# uncomment the line below for atmega168 based Diecimila
	# avrdude -p m328p -P /dev/ttyUSB0 -c stk500v1 -b 19200 -F -u -U flash:w:$(PROG).rom

fuse:
	avrdude -p atmega168 -c stk200 -U lfuse:w:0x62:m

# command to clean up junk (no source files) (invoked by "make clean")
clean:
	rm -f *.o *.rom *.elf *.map *~ *.lst

