
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding 

ASM = nasmw
AFLAGS = -f coff

LD = ld
LFLAGS = -r --cref


OBJDIR = ../../obj
SRCDIR = ../../src
INCDIR = ../../include

OBJECTS = syscalls.o syscalla.o sys_mem.o sys_proc.o sys_con.o sys_time.o


all : syscalls.a $(OBJECTS)

#Linkeo Parcial  (-r)
syscalls.a: $(OBJECTS)
	$(LD)  $(LFLAGS) $(OBJECTS) -o $(OBJDIR)/syscalls.a > syscalls.map


syscalla.o: syscalls.asm
	$(ASM) $(AFLAGS) syscalls.asm -o syscalla.o

syscalls.o: syscalls.c $(INCDIR)/syscalls.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) syscalls.c -o syscalls.o

sys_mem.o: sys_mem.c $(INCDIR)/syscalls.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) sys_mem.c -o sys_mem.o

sys_proc.o: sys_proc.c $(INCDIR)/syscalls.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) sys_proc.c -o sys_proc.o

sys_con.o: sys_con.c $(INCDIR)/syscalls.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) sys_con.c -o sys_con.o

sys_time.o: sys_time.c $(INCDIR)/syscalls.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) sys_time.c -o sys_time.o

clean:
	del  *.o


