/* io.c */

#include "../include/system.h"


unsigned char inportb(word puerto )
{
unsigned char valor;

__asm__ __volatile__("inb %w1,%b0" : "=a" (valor) : "d" (puerto) );
return valor;
}

void outportb (word puerto, unsigned char dato)
{
__asm__ __volatile__("outb %b0,%w1" : : "a" (dato), "d" (puerto) );
}



unsigned char inportb_p(word puerto )
{
unsigned char valor;

__asm__ __volatile__("inb %w1,%b0\n\tnop" : "=a" (valor) : "d" (puerto) );
return valor;
}

void outportb_p (word puerto, unsigned char dato)
{
__asm__ __volatile__("outb %b0,%w1\n\tnop" : : "a" (dato), "d" (puerto) );
}
