/*!  \addtogroup Syscalls
	\page syscalls.c
	Core de las llamadas al sistema (junto con syscalls.asm).
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
#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas
extern int sys_process (void);
extern int sys_console (void);
extern int sys_timer (void);
extern int sys_mem (void);
extern int sys_misc (void);		//Grupo de llamadas al sistema provisorias (cualquier fruta... va acá).

extern task_struct_t *actual;


// Vector de grupos de llamadas al sistema
int (*syscall_group_vector[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_process,	
	(int (*) (void)) sys_console,	
	(int (*) (void)) NULL,				// Llamada a sistema para el grupo de FLOWS
	(int (*) (void)) sys_timer,	
	(int (*) (void)) sys_mem,	
	(int (*) (void)) sys_misc	
};

// Definen la cantidad máxima de llamadas al sistema por grupo. Son pasadas a una variable para poder levantar
// su valor desde syscalls.asm

unsigned int syscall_group_max[MAX_SYSCALLS] = {
	SYS_PROCESS_MAX,
	SYS_CONSOLE_MAX,
	0,
	SYS_TIMER_MAX,
	SYS_MEM_MAX,
	SYS_MISC_MAX
};

int sys_no_existe (dword numero)
{
    kprintf("SysCall Inexistente: Grupo %d\tLlamada %d\n", numero>>16 , (word)numero);
    actual->err_no = ENOSYS;
    return -1;
}	

