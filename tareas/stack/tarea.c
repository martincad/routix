// Usada para pruebas "destructivas"
#include "../include/routix.h"

int buff[50];
char str[]="Mi nombre es martin  ";

void pepito(int,int,int);
char a[10];
void main(void) 
{
	printf("Antes de ejecutar setvar\n");
	setvar("Martino",100);
	setvar("Martino",100);

	printf("GETVAR pepe: %d\n", getvar("pepe"));
	printf("GETVAR Martino: %d\n", getvar("Martino"));
}    

void pepito(int a, int b, int i)
{
    i++;
    if (i<200)
        pepito(0,0,i);
}
