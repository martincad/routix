/* routix.c */
#include "stdarg.h"
#include "routix.h"
#include "syscalls.h"
void sprintn ( unsigned int num, int base);
void sputchar (char car);
char getascii (char c);

#define _syscall0(numero,retorno) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero))
#define _syscall1(numero,retorno,param1) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero), "b" (param1))
#define _syscall2(numero,retorno,param1,param2) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero), "b" (param1), "c" (param2))


int sleep(int segundos)
{
  int retorno;

 _syscall1( SYS_TIMER | SYS_SLEEP, retorno, segundos);
  
 return retorno;
}

int exec (char *tarea)
{
    __asm__ __volatile__ ("movl %0, %%eax ; movl %1, %%ebx" : : "g" (SYS_PROCESS | SYS_EXEC) , "g" (tarea) : "eax" , "ebx");
    __asm__ __volatile__ ("int $0x50");

     int retorno;
    __asm__ __volatile__ ("movl %%eax, %0" : "=g" (retorno)  );
 
    return retorno;
}	


void gets (char *str)
{

    __asm__ __volatile__ ("movl %0, %%eax ; movl %1, %%ebx" : : "g" (SYS_CONSOLE | SYS_GETS) , "g" (str) : "eax" , "ebx" );
    __asm__ __volatile__ ("int $0x50");

}	

int fork(void)
{
    __asm__ __volatile__ ("movl %0, %%eax" : : "g" (SYS_PROCESS | SYS_FORK) : "eax");
    __asm__ __volatile__ ("int $0x50");
}	


void voido (void)
{
    __asm__ __volatile__ ("movl %0, %%eax" : : "g" (SYS_PROCESS | SYS_VOID) : "eax");
    __asm__ __volatile__ ("int $0x50");
    
}	

// Funcion de libraria "putchar"
int putchar (char car)
{
    char aux[2];
    aux[1]='\0';
    aux[0]=car;
    puts(aux);
}	

// llamada al sistema (similar a write pero hacia stdout)
void puts(char *str)
{

    __asm__ __volatile__ ("movl %0, %%eax ; mov %1, %%ebx ; mov %2, %%ecx" : : \
		    "g" (SYS_CONSOLE | SYS_PRINT), "g" (str), "g" (strlen(str)) : "eax", "ebx", "ecx" );

    __asm__ __volatile__ ("int $0x50");
}	

int clrscr(void)
{
    __asm__ __volatile__ ("movl %0, %%eax" : : "g" (SYS_CONSOLE | SYS_CLRSCR));
    __asm__ __volatile__ ("int $0x50");
}	

#define MAX_STRING  100

word sposicion=0;
char string_stdio[MAX_STRING];

void sputchar (char car)
{
    string_stdio[sposicion] = car;
    string_stdio[++sposicion] = '\0';
}	

void printf ( char *stri, ...)
{
    string_stdio[0]='\0';    
    char *p=stri;
    char *d;
    char nibble[8];
    char car;

    unsigned int i,flag;
	
    va_list argumentos;

    va_start(argumentos, stri );

    for ( p=stri; *p ; p++ ) {
	if ( *p != '%' ) {
	    sputchar(*p);
	    continue;
	}
  
	switch (*++p) {

	case 'c':
	     car=va_arg(argumentos, int);     
	     sputchar( (char) car);
	     break;
	
        case 'x':
	     i = va_arg(argumentos, unsigned int );
	     sprintn(i,16);
	     break;

	case 'd':
	     i = va_arg(argumentos, unsigned int);
	     sprintn(i,10);
	     break;

	case 'o':
	     i = va_arg(argumentos, unsigned int);
	     sprintn(i,8);
	     break; 

	case 's':
	     d = va_arg(argumentos, char *);
	     strcat(string_stdio, d);
	     sposicion = sposicion + strlen(d);
	     break;
	
     	default:
	     sputchar(*p);
	     break;
	}
	
    }
  
    va_end(argumentos);
    puts(string_stdio);
}



void sprintn( unsigned int num, int base)
{
    unsigned int div;
    if ( (div=num/base) ) 
	sprintn(div,base);
    sputchar( getascii(num%base) );
}




//****************************************************************************************************
// printf ( char *string, ...)  y funciones requeridas por ella
//****************************************************************************************************
/*
char getascii_ ( char c );
void printn_ ( unsigned int num, int base);

void printf ( char *string, ...)
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
	     printn_(i,16);
	     break;

   case 'd':
	     i = va_arg(argumentos, unsigned int);
	     printn_(i,10);
	     break;

   case 'o':
	     i = va_arg(argumentos, unsigned int);
	     printn_(i,8);
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

void printn_ ( unsigned int num, int base)
{
 unsigned int div;
 if ( (div=num/base) ) printn_(div,base);
 putchar( getascii_(num%base) );
}
*/
char getascii ( char c )
{
 char valor = '0' + c;

 if ( valor > '9' ) valor += 7;
 return valor;
}


