
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding 

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = teclado.o video.o


all : console.a teclado.o video.o

#Linkeo Parcial  (-r)
console.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/console.a > console.map


teclado.o: teclado.c $(INCDIR)/system.h $(INCDIR)/event.h
	$(CC) $(CFLAGS) teclado.c -o teclado.o

video.o: video.c $(INCDIR)/system.h $(INCDIR)/stdio.h
	$(CC) $(CFLAGS) video.c -o video.o


clean:
	del  *.o


