#Makefile relacionado al switcheo de tareas

CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding -Wall -I$(INCDIR)

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = floppy.o


all : floppy.a

#Linkeo Parcial  (-r)
floppy.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/floppy.a > floppy.map


floppy.o: floppy.c $(INCDIR)/system.h $(INCDIR)/drivers/floppy.h
	$(CC) $(CFLAGS) floppy.c -o floppy.o

clean:
	del  *.o


