/* Funciones de video grales */

#include "../../include/system.h"
#include "../../include/stdio.h"
#include <stdarg.h>

#define VIDEO	0xb8000

#define TAB_SPACES 4

unsigned char *pos=(unsigned char *) VIDEO;
unsigned char cursor_caracteristicas = 2;


int putchar ( int a )
{

// __asm__ __volatile__("cli");

 if ( a == '\n' ) {
  int frac;
  int newpos;
  frac = (int) (pos-VIDEO)/160;
  newpos = (VIDEO+2*80*(frac+1)); 
  if ( newpos >= VIDEO+4000 ) {
	  scroll_up();
  	  pos = (unsigned char *) ( VIDEO + 80*24*2 );
  }
  else { pos=(unsigned char *) newpos; }
 }
 else if ( a == '\t' ) {
  int i;
  for (i=0; i<4; i++ ) pos=nextpos(pos);
 }
 else if ( a == '\b' ) {
   pos-=2;
   *pos=' ';
   *(pos+1) = cursor_caracteristicas;
 }

 else { 
  *pos = (unsigned char) a;
  *(pos+1) = cursor_caracteristicas;
  pos=nextpos(pos);
 }

// __asm__ __volatile__("sti");
 
 return a;
}




int puts ( const char *s )
{
 int cantidad=0;
 
 while ( *s ) {
  putchar(*s++);
  cantidad++;
 }

 return cantidad;
}




unsigned char *nextpos ( unsigned char *posicion )
{
 unsigned char *pos=posicion;
 
 pos += 2;
	
 if ( pos >= (unsigned char *) VIDEO+4000 ) {
  scroll_up();
  pos = (unsigned char *) ( VIDEO + 80*24*2 );
 }

 
 return pos;
}





void clrscr (void)
{
 int i;
 unsigned char *ptr;
 
 ptr = (unsigned char *) VIDEO;

 for (i=0; i<4000; i+=2 ) {
  *(ptr+i) = ' ';
  *(ptr+i+1) = 2;
 }

 pos=(unsigned char *) VIDEO;
}



void scroll_up ( void )
{
 unsigned char *inicio =(unsigned char *) VIDEO+80*2;
 unsigned char *destino=(unsigned char *) VIDEO;
 int count;

 __asm__ __volatile__("pushf\n\tcli");

 for ( count=0; count < 4000-80*2; count++ ) {
  *destino++ = *inicio++;
 }

 for ( count=0; count < 80; count ++ ) {
  *destino++ = ' ';
  *destino++ = cursor_caracteristicas;
 }

 __asm__ __volatile__("popf");

}




unsigned char *gotoxy ( int fila, int columna)
{
 if ( (fila > 25) || (columna > 80) ) { return; }
 pos = (unsigned char *) (VIDEO + (fila-1)*2*80+2*(columna-1) );
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
 putchar( getascii(num%base) );
}





char getascii ( char c )
{
 char valor = '0' + c;

 if ( valor > '9' ) valor += 7;
 return valor;
}

