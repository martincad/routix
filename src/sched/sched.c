/*!  \addtogroup Scheduler
	\page sched.c
	Contiene la base del scheduler y la tarea Init
*/


#include "../../include/system.h"
#include "../../include/segm.h"
#include "../../include/paging.h"
#include "../../include/kalloc.h"
#include "../../include/task.h"
#include "../../include/debug.h"
#include "../../include/stdio.h"

void idle_task(void);
extern void shell(void);
extern void rotor(void);
extern void rotor2(void);
extern void test(void);

void tarea_init(void);	    //Tarea INIT

// Puntero a la lista enlazada de tareas
task_struct_t *tareas_inicio=NULL;

// Puntero a la tarea actual
task_struct_t *actual=NULL;

void start_scheduler(void)
{
 	
   
 /* Por ahora usamos la memoria fisica directamente, total esta mapeada
  * hasta que reubiquemos el codigo y datos de las tareas a una dirección
  * virtual particular */
 
 // Inicializo el tss con valores nulos
 inicializarTss(&tss, 0, 0, 0, 0, 0 );	
 
 // Tarea fundamental del sistema ! INIT_TASK corre cuando no hay nada para correr
 task_struct_t *init_task;
 addr_t INIT_stack = kmalloc_page();
 
 init_task=init_new_task(DESC_CODE, DESC_DATA, (dword) &tarea_init, INIT_stack + 4096, 0x202, "init", 1);
 
 // Despertamos la nueva tarea, ya que recordemos init_new_task crea la nueva tarea en estado STOPPED
 despertar_task(init_task);
 
 // La tarea en ejecución será init_task
 actual = tareas_inicio;
 
}



void tarea_init()
{
    kprintf("ejecutando INIT.....\n");

    if ( exec("shell.bin")!=OK )
	perror("shell");

    // El código siguiente solo corre cuando no hay nada
    // por ejecutarse (ninguna tarea en estado TASK_RUNNING)
    
    while(1);

}	


/* Pasa los scan codes del buffer en crudo, a los ASCII con combinaciones de ALT, SHIFT y CTRL */
extern void leer_buff_teclado (void);


// Busca la próxima tarea activa
void scheduler()
{
 task_struct_t *tmp;

 leer_buff_teclado();
 
 // Volvemos al principio
 if (  ((tmp = actual->proxima ) == NULL)  ) { tmp = tareas_inicio; }

 // Buscamos una tarea para correr
 while ( tmp->estado != TASK_RUNNING ) {
    if ( (tmp = tmp->proxima) == NULL ) { tmp = tareas_inicio; }
 }
 actual = tmp;
}



