/* pic.c */

#include "../include/system.h"

#define PIC1_A0	0x20
#define PIC1_A1	0x21
#define PIC2_A0	0xa0
#define PIC2_A1	0xa1

void inicializarPics(unsigned char int1, unsigned char int2)
{

// Inicializamos con ICW1
outportb(PIC1_A0,0x11);
outportb(PIC2_A0,0x11);

// Movemos las interrupciones (ICW2)
outportb(PIC1_A1,int1);
outportb(PIC2_A1,int2);

// Seteamos la posicion del PIC slave (ICW3)
outportb(PIC1_A1,0x4);
outportb(PIC2_A1,0x2);

// Otros seteos: EOI, modo 8086 (ICW4)
outportb(PIC1_A1,0x1);
outportb(PIC2_A1,0x1);

// Deshabilitamos todas las interrupciones
outportb(PIC1_A1,0xff);
outportb(PIC2_A1,0xff);

}



void mascarasPics(unsigned char mask1, unsigned char mask2)
{
outportb(PIC1_A1,mask1);
outportb(PIC2_A1,mask2);
}



void endOfInterrupt(void)
{
outportb(PIC1_A0,0x20);
}

void enable_irq (byte irqn)
{
    word picn= PIC1_A1;
    if ( irqn > 7)
    {
    	    picn = PIC2_A1;
    }
    outportb (picn, inportb(picn ) &  ~ ( 1 << irqn) );
 
}	

void disable_irq (byte irqn)
{
    word picn= PIC1_A1;
    if ( irqn > 7)
    {
    	    picn = PIC2_A1;
    }
    outportb (picn, inportb(picn ) | ( 1 << irqn) );
 
}	

