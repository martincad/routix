
CC = gcc
CFLAGS = -c -ffreestanding -fwritable-strings 

ASM = nasmw
AFLAGS = -f aout

LD = ld
LFLAGS = --script tarea.ld --print-map --format coff-go32 -s 


SRCDIR = src
INCDIR = include
LIBDIR = ../lib

OBJECTS = tarea.o $(LIBDIR)/routix.o $(LIBDIR)/string.o

ps.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o ps.bin  > ps.map

tarea.o: ps.c
	$(CC) $(CFLAGS) ps.c -o tarea.o

$(LIBDIR)/routix.o:
$(LIBDIR)/string.o:



clean:
	del *.o
	del *.bin
	del *.s
	del *.map

