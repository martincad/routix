/*!  \addtogroup Syscalls
	\page sys_time.c
	Funciones de control y administración de tiempo y timers.
*/


#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/elf.h"
#include "../../include/file.h"
#include "../../include/errno.h"
#include "../../include/timer.h"
#include "../../include/atomic.h"

#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas
extern int sys_process (void);
extern int sys_console (void);
extern int sys_timer (void);
extern int sys_mem (void);

extern task_struct_t *actual;




// Vector de funciones de llamadas al sistema (grupo Timer)
int (*syscall_timer[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_sleep,
	(int (*) (void)) sys_proc_dump,
	(int (*) (void)) sys_kill,
	(int (*) (void)) sys_usleep,
	(int (*) (void)) sys_proc_dump_v
};



int sys_sleep(int segundos)
{
 sys_usleep(segundos*1000000);
}

int sys_usleep(int usegundos)
{
 timer_t *timer;

 // Creamos espacio en mem para la structura del timer
 timer = (timer_t *) malloc( sizeof(timer_t) );

// timer = &timer_array[timer_index++];

 // Lo llenamos
 timer->ticks = usegundos / 10000;
 timer->proceso = actual;

 // Insertamos el timer
 insertar_timer(timer);
 
 // Dormimos la tarea actual
 dormir_task(actual);

 // Switcheamos de proceso mediante la macro _reschedule quien realiza una llamada
 // a la interrupción 0x51
 _reschedule();

 // Volvemos ! el timer se venció. Este es nuestro punto de retorno.
 // todavía estamos en modo kernel en la llamada al sistema

 // Sacamos el timer vencido de la estructura de timers
 remover_timer(timer);
 
 // Liberamos el espacio alocado
 free(timer);

 return 0;
}



int sys_proc_dump(void)
{
 task_struct_t *tmp;

 char *estados[] = { "TASK_RUNNING", "TASK_STOPPED", "TASK_INTERRUMPIBLE", "TASK_ININTERRUMPIBLE", "TASK_ZOMBIE", \
					"TASK_CLEAN" };

 kprintf("pid ppid wait descripcion estado pri cuenta cpu senpend\n");
 for (tmp = tareas_inicio; tmp != NULL ; tmp=tmp->proxima ) {

  kprintf("%d\t%d\t%d\t%s", tmp->pid, tmp->ppid, tmp->wait_child, tmp->descripcion);
  kprintf(" %s %d %d %d",estados[tmp->estado], tmp->prioridad,tmp->cuenta,tmp->tiempo_cpu);
  kprintf(" 0x%x\n", tmp->senales.senales_pendientes);

 }

 return 0;
}

// Sys Proc Dump Verbose
int sys_proc_dump_v(int pid)
{
    int i;
    task_struct_t *tmp;
    //Ubicar el pid en la lista de procesos

    if ( (tmp=encontrar_proceso_por_pid(pid))==NULL ) {
		actual->err_no = ESRCH;	    //proceso no existe
        return -1;
    }

	if (tmp->estado == TASK_ZOMBIE) { //Si el proceso es Zombie, no tiene que mostrar
		actual->err_no = ESRCH;	    
		return -1;
	}
	
    kprintf("Descripcion: %s\tPID: %d\n", tmp->descripcion, tmp->pid);
    kprintf("Memoria utilizada: ");
    kprintf("Codigo: %d\tDatos: %d\tStack: %d", tmp->num_code, tmp->num_data, tmp->num_stack);
    kprintf("\nCR3: 0x%x\t", tmp->cr3_backup);
    kprintf("Stack Kernel: 0x%x\t", tmp->esp0 & 0xfffff000);
    struct user_page *mem;
    kprintf("\nPaginas Codigo:\n");
    for(mem=tmp->mcode ; mem ; mem=mem->next)
	kprintf("Direccion: 0x%x\tDireccion logica: 0x%x\n", mem->dir, mem->vdir);
    kprintf("Paginas Datos:\n");
    for(mem=tmp->mdata ; mem ; mem=mem->next)
	kprintf("Direccion: 0x%x\tDireccion logica: 0x%x\n", mem->dir, mem->vdir);
    kprintf("Paginas Stack:\n");
    for(mem=tmp->mstack ; mem ; mem=mem->next)
	kprintf("Direccion: 0x%x\tDireccion logica: 0x%x\n", mem->dir, mem->vdir);

    return OK;
}	
    

