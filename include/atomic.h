/* atomic.h */

#ifndef __SYSTEM
#include "system.h"
#endif

typedef volatile int spinlock_t;

int TestAndSet(spinlock_t *);

void spin_lock (spinlock_t *);
void spin_unlock (spinlock_t *);

