/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"
#include "../../include/debug.h"

int main (void) 
{
	
 int i;
 unsigned char *ptr = (unsigned char* ) 0xb8000 + 79*2;

 char simbolos[] = { '|', '/', '-', '\\' };

 while ( 1 ) {
	
   for ( i=0; i<sizeof(simbolos); i++) {
     *ptr = simbolos[i];
     *(ptr+1) = 0x5f;
     usleep(125000);
   }

 }

}
