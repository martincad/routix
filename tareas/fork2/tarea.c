/* tarea.c */
/* Prueba de que los espacios reservados via malloc son copiados a los hijos */
#include "stdarg.h"
#include "../include/routix.h"

int buff[50];

void main(void) 
{
	
    int cant = sizeof(int);
    int *buff = (int *) malloc(4 * cant); 
    *buff = 100;
    printf("BUFF vale: %d\n", *buff);
    int pid = fork();
    if (pid==0) {   //HIJO
	*buff += 100;
        printf("HIJO: BUFF deberia valer 200: %d\n", *buff);
	printf("\nHIJO: Mi pid es: %d\tEl de mi padre: %d\n", get_pid(),get_ppid());
    }
    else {	    //PADRE
	*buff += 400;
        printf("PADRE: BUFF deberia valer 500: %d\n", *buff);
    }
    while(1);
}

