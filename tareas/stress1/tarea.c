/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"

char msg[40];

void main(void) 
{
	int i=0;
	int j=1;

	for (j=1 ; j<=10 ; j++) {
	
		for( i=0 ; i<0x1fffff ; i++);
		exec("rotor1.bin");
		for( i=0 ; i<0x1fffff ; i++);
		exec("rotor2.bin");
		for( i=0 ; i<0x1fffff ; i++);
		exec("rotor3.bin");
		for( i=0 ; i<0x1fffff ; i++);
		exec("rotor4.bin");

		printf("\nStress info: se ha logrado ejecutar %d procesos\n", 4 * j);
	}
    
    while(1);
}

