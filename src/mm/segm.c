/*!  \addtogroup MemoryManager
	\page segm.c
	Encargado de la segmentación. Como Routix basa su esquema de memoria en la paginación, este módulo
	es sólo utilizado al inicializar el sistema.
*/


#include "../../include/segm.h"

#ifndef __SYSTEM
#include "../../include/system.h"
#endif


descriptor_t make_descriptor( dword base, dword limite, byte atr1, byte atr2 )
{
 descriptor_t gdt_entry;

 gdt_entry.base1 = (word) (base & 0Xffff);
 gdt_entry.base2 = (byte) ((base & 0xff0000) >> 16);
 gdt_entry.base3 = (byte) ((base & 0xff000000) >> 24);
 
 gdt_entry.limite1 = (word) (limite & 0xffff);
 gdt_entry.atr2    = (byte) (atr2 | ((limite & 0xf0000) >> 16));
 gdt_entry.atr1	   = atr1;

 return gdt_entry;
}

descriptor_t make_int_gate( word segselector, dword offset, byte atr)
{
 descriptor_t int_gate;

 int_gate.base1   = segselector;
 int_gate.limite1 = (word) (offset & 0xffff);
 int_gate.atr1    = (byte) (atr | INT_GATE);
 int_gate.atr2    = (byte) ((offset & 0xff0000) >> 16);
 int_gate.base3   = (byte) ((offset & 0xff000000) >> 24);
 
 return int_gate;
}

descriptor_t make_trap_gate( word segselector, dword offset, byte atr)
{
 descriptor_t int_gate;

 int_gate.base1   = segselector;
 int_gate.limite1 = (word) (offset & 0xffff);
 int_gate.atr1    = (byte) (atr | TRAP_GATE);
 int_gate.atr2    = (byte) ((offset & 0xff0000) >> 16);
 int_gate.base3   = (byte) ((offset & 0xff000000) >> 24);
 
 return int_gate;
}


descriptor_t make_task_gate( word segselector, byte atr)
{
 descriptor_t task_gate;

 task_gate.base1   = segselector;
 task_gate.atr1    = (byte) (atr | TASK_GATE);
 
 return task_gate;
}



void clear_busy_bit(descriptor_t *desc)
{
 desc->atr1 = desc->atr1 & ~2;
}



__inline__ void lgdt (gdtr_t *dir_gdtr_so)
{
 __asm__("lgdt (%0)" : : "q" (dir_gdtr_so) );
}

__inline__ void lidt (gdtr_t *dir_gdtr_so)
{
 __asm__("lidt (%0)" : : "q" (dir_gdtr_so) );
}

