/* tarea.c */
/* Prueba de que los espacios reservados via malloc son copiados a los hijos */
#include "stdarg.h"
#include "../include/routix.h"

int buff[50];

void main(void) 
{
	int pid;
	int salida, ret;
	pid=fork();

	if (pid==0) {	//Hijo
		printf("Hijo Durmiendo...\n");
		usleep(0x2fffffff);
		printf("Hijo despertando... y saliendo con 10\n");
		exit(10);
	}	
	else if (pid==-1)
		perror("fork");
	else {
		printf("Padre: pid de mi hijo: %d\n", pid);
		printf("Padre: llamdo a wait...\n");
		ret = wait(&salida);
		printf("Hijo que termino: %d\n", ret);
		printf("Salida del hijo: %d\n", salida);
		while(getvar("pepe")!=1);
	}
}

