/* atomic.h */

#ifndef __SYSTEM
#include "system.h"
#endif

typedef int spinlock_t;

int TestAndSet(volatile int *);

void spin_lock (volatile spinlock_t *);
void spin_unlock (volatile spinlock_t *);

