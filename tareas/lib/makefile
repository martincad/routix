
CC = gcc
CFLAGS = -c -ffreestanding -fwritable-strings -fpack-struct -Wall -I$(INCDIR)

ASM = nasmw
AFLAGS = -f aout

OBJDIR = obj
SRCDIR = src
INCDIR = ../include

RoutixStd = RoutixStd
Str = string
Alloc = alloc

OBJECTS = routix.o string.o alloc.o

all : routstd.a $(OBJECTS) 
	
#Crear "libreria" routstd.a (Routix Standar)
routstd.a: $(OBJECTS)
	ar r routstd.a $(OBJECTS)

#Routix-Standar
routix.o: $(RoutixStd)/routix.c
	$(CC) $(CFLAGS) $(RoutixStd)/routix.c -o routix.o

#String
string.o: $(Str)/string.c
	$(CC) $(CFLAGS) $(Str)/string.c -o string.o

#Alloc
alloc.o: $(Alloc)/alloc.c
	$(CC) $(CFLAGS) $(Alloc)/alloc.c -o alloc.o

clean:
	rm *.a
	rm *.o
	del *.a
	del *.o
