/* atomic.h */

#ifndef __SYSTEM
#include "system.h"
#endif

typedef volatile int spinlock_t;

int TestAndSet(spinlock_t *);

void spin_lock (spinlock_t *);
void spin_unlock (spinlock_t *);

// Operaciones basicas atomizadas
inline int atomic_inc (int *p);				// Incremento 
inline int atomic_dec (int *p);				// Decremento 
inline int atomic_asign (int *p, int x);	// Asignacion
inline int atomic_add (int *p, int x);		// Suma
inline int atomic_sub (int *p, int x);		// Resta

