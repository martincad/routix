/* routix.c */
#include "stdarg.h"
#include "routix.h"
#include "syscalls.h"
void sprintn ( unsigned int num, int base);
void sputchar (char car);
char getascii (char c);

#define _syscall0(numero,retorno) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero))
#define _syscall1(numero,retorno,param1) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero), "b" (param1))
#define _syscall2(numero,retorno,param1,param2) __asm__ __volatile__ ("int $0x50" : "=a" (retorno) : "a" (numero), \
	"b" (param1), "c" (param2))


int sleep(int segundos)
{
  int retorno;

 _syscall1( SYS_TIMER | SYS_SLEEP, retorno, segundos);
  
 return retorno;
}


int usleep(int usegundos)
{
  int retorno;

 _syscall1( SYS_TIMER | SYS_USLEEP, retorno, usegundos);
  
 return retorno;
}


int proc_dump(void)
{
 int retorno;
 
 _syscall0( SYS_TIMER | SYS_PROC_DUMP, retorno);

 return retorno;
}

int proc_dump_v(int pid)
{
 int retorno;
 
 _syscall1( SYS_TIMER | SYS_PROC_DUMP_V, retorno, pid);

 return retorno;
}


//Entrega una pagina al espacio de usuario
void *malloc_page(void)
{
    void *retorno;
    _syscall0( SYS_MEM | SYS_MALLOC_PAGE, retorno);
    return retorno;
}

int free_page(void *dir)
{
    int retorno;
    _syscall1( SYS_MEM | SYS_FREE_PAGE, retorno, dir);
    return retorno;
}

size_t free_mem(void)
{
    size_t retorno;
    _syscall0( SYS_MEM | SYS_FREE_MEM, retorno);
    return retorno;
}

void perror (char *str)
{
    int retorno;
    _syscall1(SYS_PROCESS | SYS_PERROR, retorno, str);
}

typedef word pid_t;

int kill( word pid, int sig)
{
 int retorno;

 printf("kill: pid=%d sig=%d\n",pid,sig);

 _syscall2( SYS_TIMER | SYS_KILL, retorno, pid, sig);

 return retorno;
}

int exec (char *tarea)
{
     int retorno;
    _syscall1(SYS_PROCESS | SYS_EXEC, retorno, tarea);

    return retorno;
}	


void gets (char *str)
{
    int retorno;
    _syscall1(SYS_CONSOLE | SYS_GETS, retorno, str);
}	

int fork(void)
{
    int retorno;
    _syscall0(SYS_PROCESS | SYS_FORK, retorno);
}	


void voido (void)
{
    int retorno;
    _syscall0(SYS_PROCESS | SYS_VOID, retorno);
}	

int renice(word pid, word prioridad)
{
 int retorno;

 _syscall2( SYS_PROCESS | SYS_RENICE, retorno, pid, prioridad);

 return retorno;
}

pid_t get_pid (void)
{
    pid_t retorno;
    _syscall0( SYS_PROCESS | SYS_GET_PID, retorno);
    return retorno;
}

pid_t get_ppid (void)
{
    pid_t retorno;
    _syscall0( SYS_PROCESS | SYS_GET_PPID, retorno);
    return retorno;
}

void exit(int valor)
{
	int retorno;
	_syscall1(SYS_PROCESS | SYS_EXIT, retorno, valor);
}

void show(int valor)
{
	int retorno;
	_syscall1(SYS_PROCESS | SYS_SHOW, retorno, valor);
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
    int retorno;
    _syscall2(SYS_CONSOLE | SYS_PRINT, retorno, str, strlen(str));
}	

int clrscr(void)
{
    int retorno;
    _syscall0(SYS_CONSOLE | SYS_CLRSCR, retorno);
}	

#define MAX_STRING  100

word sposicion=0;



//****************************************************************************************************
// printf ( char *string, ...)  y funciones requeridas por ella
//****************************************************************************************************

//char getascii_ ( char c );
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
 putchar( getascii(num%base) );
}

char getascii ( char c )
{
 char valor = '0' + c;

 if ( valor > '9' ) valor += 7;
 return valor;
}

////////////////// Grupo Misc ///////////////////////////
int setvar (char *nombre, int valor)
{
    int retorno;
    _syscall2( SYS_MISC | SYS_SETVAR, retorno, nombre, valor);
    return retorno;
}

int getvar (char *nombre)
{
    int retorno;
	 _syscall1( SYS_MISC | SYS_GETVAR, retorno, nombre);
    return retorno;
}

