
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding 

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = init.o kmain.o

all : bootstrap.a $(OBJDIR)/boot.o init.o kmain.o

#Linkeo Parcial  (-r)
bootstrap.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/bootstrap.a > bootstrap.map

#El Boot.o lo mando a OBJ/ directamente ya que lo necesita el ldscript (no recuerdo porque y no me importa).
#seguro que esto lo arregla mariano :-)
$(OBJDIR)/boot.o: boot.asm
	$(ASM) $(AFLAGS) boot.asm -o $(OBJDIR)/boot.o

init.o: init.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) init.c -o init.o

kmain.o: kmain.c $(INCDIR)/system.h $(INCDIR)/paging.h $(INCDIR)/debug.h $(INCDIR)/segm.h
	$(CC) $(CFLAGS) kmain.c -o kmain.o


clean:
	del  *.o


