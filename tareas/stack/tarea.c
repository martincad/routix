// Usada para pruebas "destructivas"
#include "../include/routix.h"

int buff[50];
char str[]="Mi nombre es martin  ";

void pepito(int,int,int);
char a[10];
void main(void) 
{
	printf("Antes de ejecutar exit implicitamente\n");
}    

void pepito(int a, int b, int i)
{
    i++;
    if (i<200)
        pepito(0,0,i);
}
