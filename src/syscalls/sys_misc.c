/* sys_mem.c: llamadas al sistema del grupo SYS_MEM */

#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/errno.h"
#include "../../include/kalloc.h"
#ifndef __TASK
#include "../../include/task.h"
#endif

extern task_struct_t *actual;

void sys_setvar(char *nombre, int valor);
int sys_getvar(char *nombre);

// Vector de funciones de llamadas al sistema (grupo Misc)
int (*syscall_misc[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_setvar,
	(int (*) (void)) sys_getvar
};



#define VARIABLES_MAX	12
#define VAR_NAME_MAX	15
struct {
	char nombre[VAR_NAME_MAX + 1];
	int valor;
} variables[VARIABLES_MAX];



void sys_setvar(char *nombre, int valor)
{
    nombre = convertir_direccion (nombre , actual->cr3_backup);
	
	int i;

	for ( i=0 ; i<VARIABLES_MAX && strcmp(nombre, variables[i].nombre) ; i++);

	if (i<VARIABLES_MAX)
		kprintf("Estaba seteada\n");
	else {
//		strncpy(variables[i].nombre, nombre, VAR_NAME_MAX);
		strcpy(variables[i].nombre, nombre);
	}	
}

int sys_getvar(char *nombre)
{
    nombre = convertir_direccion (nombre , actual->cr3_backup);
	int i, valor;

	for ( i=0 ; i<VARIABLES_MAX && strcmp(nombre, variables[i].nombre) ; i++);

	if (i<VARIABLES_MAX)
		return variables[i].valor;
	
	return 778899;
}

