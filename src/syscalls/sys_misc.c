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

int sys_setvar(char *nombre, int valor);
int sys_getvar(char *nombre);

int find_var(char *nombre);
int find_empty_var(void);

// Vector de funciones de llamadas al sistema (grupo Misc)
int (*syscall_misc[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_setvar,
	(int (*) (void)) sys_getvar
};



// Aqui se definen unas estructuras para guardar los pares (Nombre, valor) de variables accesibles via
// setvar y getvar.
// Esto es solo momentaneo, no se encari�en. Utilizado para propositos de debug en tiempo real

#define VARIABLES_MAX	12
#define VAR_NAME_MAX	15
struct {
	char nombre[VAR_NAME_MAX + 1];
	int valor;
} variables[VARIABLES_MAX];


void init_var(void)
{
	int i;
	for (i=0 ; i<VARIABLES_MAX ; i++)
		variables[i].valor = -1;
}

int sys_setvar(char *nombre, int valor)
{
    nombre = convertir_direccion (nombre , actual->cr3_backup);
	int i;
	
	if ( (i=find_var(nombre)) ==-1 )	{		//Variable no esta definida. Agregarla
		if ( (i = find_empty_var()) ==-1 )	{
kprintf("SYS_SETVAR: no hay espacio libre\n");
			return -1;					//No hay un espacio libre
		}
		strncpy(variables[i].nombre, nombre, VAR_NAME_MAX);
		variables[i].valor = valor;
kprintf("SYS_SETVAR: no estaba definida... definiendola en indice: %d\n", i);
		return 0;
	}
	
	//Si lleg� aqu�, la variable esta definida en el indice "i", debo modificarla
kprintf("SYS_SETVAR: estaba definida ... modificandola en indice: %d\n", i);
	
	variables[i].valor = valor;
	return 0;
	
}

int sys_getvar(char *nombre)
{
    nombre = convertir_direccion (nombre , actual->cr3_backup);
	int i;

	if ( (i=find_var(nombre))!=-1) {
kprintf("SYS_GET_VAR: se encontro en indice: %d\n", i);
		return variables[i].valor;
	}

	return -1;
}

// Busca una variable en el array y retorna el indice en caso de exito, o -1 en caso de no encontrarse
int find_var (char *nombre)
{
	int i=0;
	for (i=0 ; i<VARIABLES_MAX ; i++)
		if (strcmp(variables[i].nombre, nombre)==0)
			return i;
			
	return -1;
}

// Busca un espacio libre para definir una variable. Devuelve el indice en caso de encontrarlo o -1 en caso contrario
int find_empty_var (void)
{
	int i=0;
	for (i=0 ; i<VARIABLES_MAX ; i++)
		if (variables[i].valor==-1)
			return i;
	return -1;
}
