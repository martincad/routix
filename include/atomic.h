/* atomic.h */

#ifndef __SYSTEM
#include "system.h"
#endif

typedef int sem_t;

int TestAndSet(volatile int *);

void sem_wait (sem_t *);
void sem_post (sem_t *);

