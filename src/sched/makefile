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

OBJECTS = task.o taska.o sched.o


all : sched.a task.o taska.o sched.o

#Linkeo Parcial  (-r)
sched.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/sched.a > sched.map


taska.o: task.asm
	$(ASM) $(AFLAGS) task.asm -o taska.o

task.o: task.c $(INCDIR)/system.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) task.c -o task.o

sched.o: sched.c $(INCDIR)/system.h $(INCDIR)/task.h $(INCDIR)/debug.h \
		       $(INCDIR)/segm.h	     $(INCDIR)/paging.h
	$(CC) $(CFLAGS) sched.c -o sched.o

clean:
	del  *.o


