// stress2.bin
// Ejecuta la tarea stack.bin mcuhas veces, en busca de memory leeks 
// o colgadas
#include "stdarg.h"
#include "../include/routix.h"

char msg[40];

void main(void) 
{
	int i, j;
	int mem_before = free_mem();
	int mem_after;
	
	for (j=0 ; j<100 ; j++) {
	
		for( i=0 ; i<0x8ffff ; i++);
		if (exec("stack.bin")==-1) {
			perror("exec");
			exit (0);
		}
			
		printf("\nStress info: se ha logrado ejecutar %d procesos", j+1);
	}
    
	for( i=0 ; i<0xfffff ; i++);
	printf("\nStress2 ha ejecutado con exito %d tareas", j);
	mem_after = free_mem();
	printf("\nPaginas libre antes: %d\tahora: %d\n", mem_before,mem_after);

	while(1);	
}

