
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding -Wall

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = signal.o


all : signals.a signal.o

#Linkeo Parcial  (-r)
signals.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/signals.a > signals.map


signal.o: signal.c $(INCDIR)/system.h $(INCDIR)/signal.h
	$(CC) $(CFLAGS) signal.c -o signal.o



clean:
	del  *.o


