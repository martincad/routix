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
	
		if (fork()==0) {
			if (exec("rotor1.bin")==-1)
				perror("exec");
			exit (-1);
		}	
		for( i=0 ; i<0x4fffff ; i++);

		if (fork()==0) {
			if (exec("rotor2.bin")==-1) 
				perror("exec");
			exit (-1);
		}	
		for( i=0 ; i<0x4fffff ; i++);

		if (fork()==0) {
			if (exec("rotor3.bin")==-1) 
				perror("exec");
			exit (-1);
		}	
		for( i=0 ; i<0x4fffff ; i++);

		if (fork()==0) {
			if (exec("rotor4.bin")==-1) 
				perror("exec");
			exit (-1);
		}	
		for( i=0 ; i<0x4fffff ; i++);
		
		printf("\nStress info: se ha logrado ejecutar %d procesos\n", j*4);
	}
    
    while(1);
}

