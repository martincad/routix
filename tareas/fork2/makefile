
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

OBJECTS = tarea.o $(LIBDIR)/routstd.a

fork2.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o fork2.bin  > tarea.map

tarea.o: tarea.c
	$(CC) $(CFLAGS) tarea.c -o tarea.o

alloc.o: alloc.c
	$(CC) $(CFLAGS) alloc.c -o alloc.o


clean:
	del *.o
	del *.bin
	del *.s
	del *.map

