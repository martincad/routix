/* spinlock.c */

#include "../include/system.h"
#include "../include/atomic.h"
#include "../include/debug.h"


// Retorna el valor que posee la variable candado, y luego la setea a 1
int TestAndSet(volatile int *candado)
{
    int retorno;
    __asm__ __volatile__ ("pushf\n\t"
			  "cli\n\t"
			  "movl %2, %0\n\t"
			  "movl $1, %1\n\t"
			  "popf"
			  : "=m" (retorno), "=m" (*candado)
			  : "r" (*candado));
		
    return retorno;    
}

/* Momentaneamente solo usadas por funciones de kernel */

// Candado
volatile int kernel_lock=0;


//Funcion de bloqueo del semaforo (equivalente a down o wait)
void spin_lock (spinlock_t *valor)
{
    while(1) {
	while (TestAndSet(&kernel_lock));

	if (*valor > 0)	{
	    (*valor)--;
	    break;
	}

	kernel_lock = 0;    
    }
    kernel_lock = 0;
}


//Funcion de desbloqueo del semaforo (equivalente a up o signal)
void spin_unlock (spinlock_t *valor)
{
    while (TestAndSet(&kernel_lock));
    (*valor)++;
    kernel_lock = 0;   
}


