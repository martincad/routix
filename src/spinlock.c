/* spinlock.c */

#include "../include/system.h"
#include "../include/atomic.h"
#include "../include/debug.h"


// Retorna el valor que posee la variable candado, y luego la setea a 1
inline int TestAndSet(volatile int *candado)
{
//    int retorno;
/*	
    __asm__ __volatile__ ("pushf\n\t"
			  "cli\n\t"
			  "movl %2, %0\n\t"
			  "movl $1, %1\n\t"
			  "popf"
			  : "=m" (retorno), "=m" (*candado)
			  : "r" (*candado));
*/
	__asm__ __volatile__ ("movl %0, %%ebx ; movl $1, %%eax ; xchg %%eax, (%%ebx)" : : "m" (candado) : "ebx");
	
  //  return retorno;    
}

/* Momentaneamente solo usadas por funciones de kernel */

// Candado, utilizado para verificar si algun proceso está evaluando el contenido de "valor"
volatile int kernel_lock = 0;


//Funcion de bloqueo del semaforo (equivalente a down o wait)
void spin_lock (volatile spinlock_t *valor)
{
    while(1) {
		// Esperar mientras algún proceso este dentro de spin_lock/spin_unlock
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
void spin_unlock (volatile spinlock_t *valor)
{
	// Esperar mientras algún proceso este dentro de spin_lock/spin_unlock
    while (TestAndSet(&kernel_lock));
    (*valor)++;
    kernel_lock = 0;   
}


