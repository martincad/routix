// stress2.bin
// Ejecuta la tarea stack.bin mcuhas veces, en busca de memory leeks 
// o colgadas
#include "stdarg.h"
#include "../include/routix.h"

char msg[40];

void main(void) 
{
	int i, j;

	for (j=1 ; j<=1000 ; j++) {
	
		for( i=0 ; i<0x8fffff ; i++);
		if (exec("stack.bin")==-1) {
			perror("exec");
			exit (0);
		}
			
		printf("\nStress info: se ha logrado ejecutar %d procesos\n", j);
	}
    
    while(1);
}

