/* timedeb.c */

/* Este archivo tiene como único fin realizar el debug de la
 * libreria time.c */

#include "../../include/task.h"

#ifndef __SYSTEM
#include "../../include/system.h"
#endif


#define cmd(comando)	( ! strcmp(msg,comando) )

// Funciones
void ps(void);

void task_debug(void) {

 char msg[80];
 while(1) {
   kprintf("task debug > ");
   gets(msg);

   if ( cmd("exit") ) { return; }
   else if ( cmd("ps") ) { ps(); }

 }

}

char *estados[] = { "TASK_RUNNING", "TASK_STOPPED" };

void ps()
{
 task_struct_t *tmp;

 kprintf("Actual: 0x%x\n",actual);

 for (tmp = tareas_inicio; tmp != NULL ; tmp=tmp->proxima ) {

  kprintf("Puntero: 0x%x - Proxima: 0x%x\n",tmp,tmp->proxima);
  kprintf("Pid: %d - Desc: '%s'\n", tmp->pid, tmp->descripcion);
  kprintf("Estado: %s - Prioridad: %d - Cuenta: %d - Tiempo Cpu: %d\n\n",estados[tmp->estado],tmp->prioridad,tmp->cuenta,tmp->tiempo_cpu);

 }

}


