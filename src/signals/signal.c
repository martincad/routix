/*!  \addtogroup Signals
	\page signals.c
	Implementaci�n en desarrollo
*/


#ifndef __SIGNAL
#include "../../include/signal.h"
#endif

#include "../../include/errno.h"
#include "../../include/task.h"

#define senal_bloqueada(proceso,sig) ( proceso->senales.senales_enmascaras & sig ) 

int sys_kill(pid_t pid, int sig)
{

  task_struct_t *proceso;

  // Buscamos el proceso seg�n el n�mero de pid
  if ( (proceso = encontrar_proceso_por_pid(pid)) == NULL ) {
    return ESRCH;
  }

  kprintf("Proceso: 0x%x -> Sig: %d\n", proceso,sig);

  /* Nos fijamos si la se�al recibida esta enmascara
  if ( senal_bloqueada(proceso, sig) ) {
    return -1;
  }
  */

  // Analizamos el estado del proceso
  // si est� en TASK_STOPPED lo despertamos
  if ( proceso->estado == TASK_STOPPED ) {
    despertar_task(proceso);
  }

  // Seteamos ahora el indicador de se�al pendiente
  proceso->senales.senales_pendientes |= sig;

}
