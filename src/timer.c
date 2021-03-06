/* timer.c */

/* Manejo de la interrupcion de timertick y timers */

#include "system.h"
#include "8254.h"
#include "8259.h"
#include "debug.h"
#include "task.h"

#include "atomic.h"

#define __TIMER
#include "timer.h"

dword jiffies=0;

// Puntero al inicio de la lista de timers
volatile timer_t *timer_inicio=NULL;

void actualizar_timers(void);

void timertick_handler()
{

 endOfInterrupt();
 
 // Incrementamos el contador de jiffies
 jiffies++;

 // Actualizamos el reloj
 if ( ! (jiffies % FINTERRUPCION) ) actualizar_reloj(jiffies);

 // Revisamos los timers
 actualizar_timers();

 // Actualizamos la cuenta del proceso actual
 actual->cuenta -= 1;
 actual->tiempo_cpu++;

 // Si se venci� la cuenta, la reestablecemos y pasamos el control al scheduler
 if ( actual->cuenta <= 0 ) {
  // Reiniciamos la cuenta
  actual->cuenta = actual->prioridad;
  scheduler();
  __asm__ ("nop");
 }

}

void actualizar_timers(void)
{
  timer_t *tmp;

  // Chequeamos si hay timers activos
  if ( timer_inicio == NULL ) {
   return;
  } 

  for ( tmp=timer_inicio; tmp != NULL; tmp = tmp->proximo ) {

    // Debe estar pendiente (ver timer.h) y haber vencido su cuenta
    if ( (tmp->estado==PENDIENTE) && (--(tmp->ticks) <= 0) ) {
	    
      // Ejecutamos la funcion establecida
      (*(tmp->func))(tmp);

      // Seteamos su estado como finalizado
      tmp->estado=FINALIZADO;
    }
		 
  }
 
}


spinlock_t timer_lock = 1;

int insertar_timer(timer_t *nuevo)
{
  // Deshabilitamos Interrupciones
  cli();
	
  timer_t *tmp;
	
  if ( nuevo == NULL ) { return 0; }

  // Nos paramos al ppio de la lista
  tmp = timer_inicio;

  if ( timer_inicio == NULL ) { timer_inicio = nuevo; }

  else {
 
    // Buscamos la �ltima tarea
    for ( tmp = timer_inicio; tmp->proximo != NULL ; tmp = tmp->proximo );

    // Nos colgamos de ella
    tmp->proximo = nuevo;
  }

  // La nueva tarea queda apuntando a NULL
  nuevo->proximo = NULL;

  // Pendiente de ejecucion
  nuevo->estado=PENDIENTE;

  // Habilitamos Interrupciones
  sti();

  return 1;
}


int remover_timer(timer_t *timer)
{
  cli();

  timer_t *tmp;

  // Es el primer timer ?
  if ( timer == timer_inicio ) {
    timer_inicio = timer->proximo;
    sti();
    return 0;
  }
 
  // Buscamos nuestro timer entonces
  for ( tmp=timer_inicio; (tmp->proximo != timer) && (tmp != NULL) ; tmp = tmp->proximo ) ;

  // Si no encontramos el timer devolvemos error
  if ( tmp == NULL ) {
    // Lo tengo que reemplazar por la constante correcta seg�n la definici�n de errno.h
    sti();
    return -1;
  }

  else {
    tmp->proximo = timer->proximo;
  }

  // Lo tengo que reemplazar por la constante correcta seg�n la definici�n de errno.h
  sti();
  return 0;

}
