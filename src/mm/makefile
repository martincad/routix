
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding -Wall

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = alloc.o mm.o segm.o kalloc.o paging.o 


all : mm.a alloc.o mm.o segm.o kalloc.o paging.o 

#Linkeo Parcial  (-r)
mm.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/mm.a > mm.map


#syscalla.o: syscalls.asm
#	$(ASM) $(AFLAGS) syscalls.asm -o syscalla.o

alloc.o: alloc.c  $(INCDIR)/paging.h $(INCDIR)/debug.h
	$(CC) $(CFLAGS) alloc.c -o alloc.o

mm.o: mm.c $(INCDIR)/system.h $(INCDIR)/paging.h $(INCDIR)/segm.h $(INCDIR)/debug.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) mm.c -o mm.o

segm.o: segm.c $(INCDIR)/system.h $(INCDIR)/segm.h
	$(CC) $(CFLAGS) segm.c -o segm.o

kalloc.o: kalloc.c $(INCDIR)/system.h $(INCDIR)/debug.h $(INCDIR)/stdio.h
	$(CC) $(CFLAGS) kalloc.c -o kalloc.o

paging.o: paging.c $(INCDIR)/paging.h $(INCDIR)/debug.h
	$(CC) $(CFLAGS) paging.c -o paging.o



clean:
	del  *.o


