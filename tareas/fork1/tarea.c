/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"

char i[1000];
void main(void) 
{
    char *p;
    int pid;
    p = (char *) malloc(100);
    strcpy(p, "Hola soy el ");
    pid=fork();
    if (pid==0)
	strcat(p, "HIJO !!!\n");
    else strcat(p, "PADRE !!!\n");

    //Imprimir tanto en PADRE como en HIJO el string apuntado por p
    printf("%s", p);    
    
    while(1);
}

