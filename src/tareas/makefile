
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding 

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = timedeb.o taskdeb.o


all : taskdeb.a timedeb.o taskdeb.o

#Linkeo Parcial  (-r)
taskdeb.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/taskdeb.a > taskdeb.map


taskdeb.o: taskdeb.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) taskdeb.c -o taskdeb.o

timedeb.o: timedeb.c $(INCDIR)/system.h $(INCDIR)/stdio.h
	$(CC) $(CFLAGS) timedeb.c -o timedeb.o


clean:
	del  *.o


