// Usada para pruebas "destructivas"
#include "../include/routix.h"

int buff[50];
char str[]="Mi nombre es martin  ";

void pepito(int,int,int);
char a[10];
void main(void) 
{
	printf("Antes de ejecutar una llamada invalida\n");
	__asm__ __volatile__ ("mov $0x0110, %eax ; int $0x50");
}    

void pepito(int a, int b, int i)
{
    i++;
    if (i<200)
        pepito(0,0,i);
}
