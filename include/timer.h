/* timer.h */

#ifndef __SYSTEM
#include "system.h"
#endif

#ifndef __TASK
#include "task.h"
#endif


#ifndef __TIMER
extern dword jiffies;
#endif

typedef struct timer_t {
  dword ticks;
  task_struct_t *proceso;
  struct timer_t *proximo;
} timer_t;

void actualizar_timers(void);
int insertar_timer(timer_t *nuevo);
int remover_timer(timer_t *timer);
