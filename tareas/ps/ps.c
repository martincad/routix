/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"
#include "../../include/debug.h"

void retardo(int j);

char str[200];

int main (void) 
{
	int aaa;	
 	aaa = sprintf(str,"Mi nombre es %s y tengo %d anos de edad. Soy un %s y tengo %d hermanos", "Martin", 23, "Ritchie", -5);
 	printf("%s", str);
	printf("Largo segun strlen: %d\n", strlen(str));
	printf("Largo segun sprintf: %d\n", aaa);

	wait(&aaa);
}
