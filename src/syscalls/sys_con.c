/* sys_con.c: llamadas del grupo CONSOLE */

#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/errno.h"
#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas
extern int sys_process (void);
extern int sys_console (void);
extern int sys_timer (void);
extern int sys_mem (void);

extern task_struct_t *actual;




// Vector de funciones de llamadas al sistema (grupo Console)
int (*syscall_console[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_print,	
	(int (*) (void)) sys_gets,	
	(int (*) (void)) sys_clrscr	
};




// Similar a llamada nix Write pero escribiendo siempre hacia stdout
int sys_print (void *buff, size_t largo)
{
    buff = convertir_direccion (buff , actual->cr3_backup);
	
    size_t aux;
    for (aux=0 ; aux<largo; aux++)
	putchar(*((char *)buff+aux));

    return largo;
}	


int sys_gets (char *str)
{
    str = convertir_direccion (str , actual->cr3_backup);

    gets (str);
    return (strlen(str));
}	

int sys_clrscr (void)
{
    clrscr();
    return OK;
}


