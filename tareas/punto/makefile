
CC = gcc
CFLAGS = -c -ffreestanding -fwritable-strings 

ASM = nasmw
AFLAGS = -f aout

LD = ld
LFLAGS = --script ../ldscript/tarea.ld --print-map --format coff-go32 -s 


OBJDIR = obj
SRCDIR = src
INCDIR = include
LIBDIR = ../lib

OBJECTS = punto.o $(LIBDIR)/routstd.a

punto.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o punto.bin  > punto.map

punto.o: punto.c
	$(CC) $(CFLAGS) punto.c -o punto.o

clean:
	del *.o
	del *.bin

