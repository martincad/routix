/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"
#include "../../include/debug.h"

int main (void) 
{
	
 unsigned int i, j;
 unsigned char *ptr = (unsigned char* ) 0xb8000 + 76*2;

 char simbolos[] = { '|', '/', '-', '\\' };

 for (j=0 ; j<200 ; j++) {
	
   for ( i=0; i<sizeof(simbolos); i++) {
     *ptr = simbolos[i];
     *(ptr+1) = 0x6f;
     usleep(125000);
   }
 }
     usleep(8250000);
     *(ptr+1) = 0x00;

}
