/* libk.c */

/* Aca van todas las funciones que inician la llamada al sistema. Usadas, por ejemplo, por INIT */

#include "../include/system.h"
#include "../include/time.h"
#include "../include/paging.h"
#include "../include/task.h"
#include "../include/syscalls.h"

int exec (char *);
int exec (char *tarea)
{
    __asm__ __volatile__ ("movl %0, %%eax ; movl %1, %%ebx" : : "g" (SYS_PROCESS | SYS_EXEC) , "g" (tarea) : "eax" , "ebx");
    __asm__ __volatile__ ("int $0x50");

    int retorno;

    __asm__ __volatile__ ("movl %%eax, %0" : "=g" (retorno)  );

    return retorno;
}	





