
CC = gcc
CFLAGS = -c -fpack-struct -ffreestanding 

ASM = nasmw
AFLAGS = -f aout

LD = ld
LFLAGS = --script cero.ld -Map kernel.map  --cref  -no-keep-memory


OBJDIR = obj
SRCDIR = src
INCDIR = include
TASKDIR = $(SRCDIR)/tareas

OBJECTS = $(OBJDIR)/boot.o	$(OBJDIR)/bootstrap.a	$(OBJDIR)/mm.a		\
  	  $(OBJDIR)/syscalls.a	$(OBJDIR)/console.a 	$(OBJDIR)/sched.a	\
	  $(OBJDIR)/signals.a   $(OBJDIR)/inta.o	$(OBJDIR)/int.o	   	\
	  $(OBJDIR)/io.o		$(OBJDIR)/taskdeb.a 	$(OBJDIR)/string.o	\
  	  $(OBJDIR)/pic.o  		$(OBJDIR)/floppy.a   	$(OBJDIR)/file.o   	\
  	  $(OBJDIR)/libk.o      $(OBJDIR)/stdlib.o	$(OBJDIR)/fat12.o  	\
 	  $(OBJDIR)/time.o      $(OBJDIR)/8254.o	$(OBJDIR)/timer.o  	\
   	  $(OBJDIR)/error.o 	$(OBJDIR)/event.o	$(OBJDIR)/spinlock.o

	  

kernel.bin: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o kernel.bin  > kernel.map

$(OBJDIR)/io.o: $(SRCDIR)/io.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/io.c -o $(OBJDIR)/io.o

$(OBJDIR)/pic.o: $(SRCDIR)/pic.c
	$(CC) $(CFLAGS) $(SRCDIR)/pic.c -o $(OBJDIR)/pic.o

$(OBJDIR)/inta.o: $(SRCDIR)/int.asm
	$(ASM) $(AFLAGS) $(SRCDIR)/int.asm -o  $(OBJDIR)/inta.o

$(OBJDIR)/int.o: $(SRCDIR)/int.c $(INCDIR)/system.h $(INCDIR)/debug.h $(INCDIR)/segm.h $(INCDIR)/paging.h $(INCDIR)/event.h
	$(CC) $(CFLAGS) $(SRCDIR)/int.c -o $(OBJDIR)/int.o

$(OBJDIR)/string.o: $(SRCDIR)/string.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/string.c -o $(OBJDIR)/string.o

$(OBJDIR)/stdlib.o: $(SRCDIR)/stdlib.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/stdlib.c -o $(OBJDIR)/stdlib.o

$(OBJDIR)/fat12.o: $(SRCDIR)/fat12.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/fat12.c -o $(OBJDIR)/fat12.o

$(OBJDIR)/file.o: $(SRCDIR)/file.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/file.c -o $(OBJDIR)/file.o

$(OBJDIR)/time.o: $(SRCDIR)/time.c $(INCDIR)/time.h $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/time.c -o $(OBJDIR)/time.o

$(OBJDIR)/8254.o: $(SRCDIR)/8254.c $(INCDIR)/8254.h $(INCDIR)/debug.h
	$(CC) $(CFLAGS) $(SRCDIR)/8254.c -o $(OBJDIR)/8254.o

$(OBJDIR)/timer.o: $(SRCDIR)/timer.c $(INCDIR)/system.h
	$(CC) $(CFLAGS) $(SRCDIR)/timer.c -o $(OBJDIR)/timer.o

$(OBJDIR)/error.o: $(SRCDIR)/error.c 
	$(CC) $(CFLAGS) $(SRCDIR)/error.c -o $(OBJDIR)/error.o

$(OBJDIR)/libk.o: $(SRCDIR)/libk.c $(INCDIR)/syscalls.h
	$(CC) $(CFLAGS) $(SRCDIR)/libk.c -o $(OBJDIR)/libk.o

$(OBJDIR)/event.o: $(SRCDIR)/event.c $(INCDIR)/event.h $(INCDIR)/task.h
	$(CC) $(CFLAGS) $(SRCDIR)/event.c -o $(OBJDIR)/event.o

$(OBJDIR)/spinlock.o: $(SRCDIR)/spinlock.c $(INCDIR)/atomic.h
	$(CC) $(CFLAGS) $(SRCDIR)/spinlock.c -o $(OBJDIR)/spinlock.o



$(OBJDIR)/boot.o $(OBJDIR)/bootstrap.a : src/bootstrap/*
	cd src/bootstrap
	make 
	cd ../..

$(OBJDIR)/mm.a: $(SRCDIR)/mm/*
	cd src/mm
	make
	cd ../..

$(OBJDIR)/console.a: $(SRCDIR)/console/*
	cd src/console
	make
	cd ../..

$(OBJDIR)/sched.a: $(SRCDIR)/sched/*
	cd src/sched
	make
	cd ../..

$(OBJDIR)/syscalls.a: $(SRCDIR)/syscalls/*
	cd src/syscalls
	make
	cd ../..

$(OBJDIR)/signals.a: $(SRCDIR)/signals/*
	cd src/signals
	make
	cd ../..

$(OBJDIR)/floppy.a: $(SRCDIR)/drivers/*
	cd src/drivers
	make
	cd ../..

#taskdeb y timedeb
$(OBJDIR)/taskdeb.a: $(SRCDIR)/tareas/*
	cd src/tareas
	make
	cd ../..



clean:
	del  $(OBJDIR)\*.o
	del  $(OBJDIR)\*.a


