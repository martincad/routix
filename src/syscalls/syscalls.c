/* int.c */

#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/elf.h"
#include "../../include/file.h"
#include "../../include/errno.h"
#include "../../include/timer.h"
#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas
extern int sys_process (void);
extern int sys_console (void);
extern int sys_timer (void);
extern int sys_mem (void);

extern task_struct_t *actual;


// Vector de grupos de llamadas al sistema
int (*syscall_group_vector[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_process,	
	(int (*) (void)) sys_console,	
	(int (*) (void)) NULL,				// Llamada a sistema para el grupo de FLOWS
	(int (*) (void)) sys_timer,	
	(int (*) (void)) sys_mem	
};



int sys_no_existe (dword numero)
{
    kprintf("Llamada no existe: %d\n", numero);
    actual->err_no = ENOSYS;
    return -1;
}	

