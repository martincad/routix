/* libk.c */

/* Aca van todas las funciones que inician la llamada al sistema. Usadas, por ejemplo, por INIT */

#include "../include/system.h"
#include "../include/time.h"
#include "../include/paging.h"
#include "../include/task.h"
#include "../include/syscalls.h"

#include <stdarg.h>

int exec (char *);
char getascii_video (char c);
void printn( unsigned int num, int base);


int exec (char *tarea)
{
    __asm__ __volatile__ ("movl %0, %%eax ; movl %1, %%ebx" : : "g" (SYS_PROCESS | SYS_EXEC) , "g" (tarea) : "eax" , "ebx");
    __asm__ __volatile__ ("int $0x50");

    int retorno;

    __asm__ __volatile__ ("movl %%eax, %0" : "=g" (retorno)  );

    return retorno;
}	


void kprintf ( char *string, ...)
{
 char *p=string;
 char *d;
 char nibble[8];
 char car;

 unsigned int i,flag;
	
 va_list argumentos;

 va_start(argumentos, string );

 for ( p=string; *p ; p++ ) {
  if ( *p != '%' ) {
   putchar(*p);
   continue;
  }
  
  switch (*++p) {

   case 'c':
	     car=va_arg(argumentos, int);     
	     putchar( (char) car);
	     break;

   case 'x':
	     i = va_arg(argumentos, unsigned int );
	     printn(i,16);
	     break;


   case 'd':
	     i = va_arg(argumentos, int);
		 if (i > (0xffffffff/2) ) {
			putchar('-');
			printn(~i+1,10);
			break;
		 }
	     printn(i,10);
	     break;

   case 'u':
	     i = va_arg(argumentos, unsigned int);
	     printn(i,10);
	     break;
		 
   case 'o':
	     i = va_arg(argumentos, unsigned int);
	     printn(i,8);
	     break; 

   case 's':
	     d = va_arg(argumentos, char *);
	     puts(d);
	     break;
	     
	     

   default:
	     putchar(*p);
	     break;

  }
	
 }
  
 va_end(argumentos);
}

void printn( unsigned int num, int base)
{
 unsigned int div;
 if ( (div=num/base) ) printn(div,base);
 putchar( getascii_video(num%base) );
}


char getascii_video ( char c )
{
 char valor = '0' + c;

 if ( valor > '9' ) valor += 7;
 return valor;
}




