// Usada para pruebas "destructivas"
#include "../include/routix.h"

int buff[50];
char str[]="Mi nombre es martin  ";

void pepito(int,int,int);

void main(void) 
{
    pepito(10,10,1);
    printf("Saliendo...");
    while(1);
}    

void pepito(int a, int b, int i)
{
    i++;
    if (i<200)
        pepito(0,0,i);
}
