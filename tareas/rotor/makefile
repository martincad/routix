
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

OBJECTS = rotor.o $(LIBDIR)/routstd.a

rotor.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o rotor.bin  > rotor.map

rotor.o: rotor.c
	$(CC) $(CFLAGS) rotor.c -o rotor.o

clean:
	del *.o
	del *.bin
	del *.s
	del *.map

