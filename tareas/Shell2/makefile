
CC = gcc
CFLAGS = -c -ffreestanding -fwritable-strings -Wall -I$(INCDIR)

ASM = nasmw
AFLAGS = -f aout

LD = ld
LFLAGS = --script ../ldscript/tarea.ld --print-map --format coff-go32 -s 


OBJDIR = obj
SRCDIR = src
INCDIR = ../include
LIBDIR = ../lib

OBJECTS = shell.o $(LIBDIR)/routstd.a 

shell.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o shell.bin  > tarea.map

shell.o: shell.c
	$(CC) $(CFLAGS) shell.c -o shell.o


clean:
	del *.o
	del *.bin
	del *.s
	del *.map

