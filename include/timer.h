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

#define PENDIENTE	0
#define FINALIZADO	1

typedef struct timer_t {
  dword ticks;
  task_struct_t *proceso;
  void (*func)(struct timer_t *info);			// Puntero a la funcion a ejecutar
  void *data;						// Puntero a datos genericos
  struct timer_t *proximo;
  byte estado;						// Indica si esta pendiente su analisis (PENDIENTE) 
  							// o no (FINALIZADO)
  							// esto es porque cuando vence un timer, luego de
							// ejecutar la funcion por el descripta, es el proceso
							// que lo genero quien lo borra (NO la funcion), entonces
							// en proximo timertick podria ejecutarse nuevamente.
} timer_t;

void actualizar_timers(void);
int insertar_timer(timer_t *nuevo);
int remover_timer(timer_t *timer);
