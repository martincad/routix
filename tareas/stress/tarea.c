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
	
	int pepe = getvar("veces");
	if (pepe==-1) {
		printf("Debe setear el valor de la variable veces. Ejecutandolo 10 veces\n");
		pepe=10;
	}
	
	for (j=0 ; j<pepe ; j++) {
	
		setvar("stress",1);
		if (exec("wait2.bin")==-1) {
			perror("exec");
			exit (0);
		}
			
		while (getvar("stress")==1)
			usleep(0xfffff);

		printf("*** Se ejecutaron: %d wait2.bin ***\n", j+1);
	}

	printf("Memoria antes: %d\tAhora: %d\n", mem_before, free_mem());
}

