/* segm.h */

#ifndef __SYSTEM
#include "system.h"
#endif

typedef struct gdtr_t {
	word limite;
	dword base;
} gdtr_t;

typedef gdtr_t idtr_t;


typedef struct descriptor_t {
	word limite1;			// Limite 0:15
	word base1;			// Base 0:15
	byte base2;			// Base 16:23
	byte atr1;			// P, DPL(2bits),S, Tipo(4bits)
	byte atr2;			// G, D/B, 0, AVL, Limite 16:19 (4bits) 
	byte base3;			// Base  24:31
} descriptor_t;


#define PRESENTE 1 << 7
#define DPL_0	 0 << 6
#define DPL_1	 1 << 5
#define DPL_2	 2 << 5
#define DPL_3	 3 << 5
#define GENERAL  1 << 4
#define SISTEMA  0

//Descriptor de tipo GENERAL
//Tipos
#define DATA	 	0 << 3
#define EXPAND_DOWN	1 << 2
#define WRITE		1 << 1


#define CODIGO	 	1 << 3
#define	CONFORMING	1 << 2
#define READ		1 << 1

#define ACCESED		1 << 0

// Descriptor de tipo SISTEMA
// Tipos
#define LDT		2
#define TASK_GATE	5
#define TSS_AVAILABLE	9
#define TSS_BUSY	0xb
#define CALL_GATE	0xc
#define INT_GATE	0xe
#define TRAP_GATE	0xf


// 2do byte
#define GRANULARIDAD	1 << 7
#define DB		1 << 6


__inline__ void lgdt (gdtr_t *dir_gdtr_so);
descriptor_t make_descriptor( dword base, dword limite, byte atr1, byte atr2 );
descriptor_t make_int_gate( word segselector, dword offset, byte atr);
descriptor_t make_trap_gate( word segselector, dword offset, byte atr);
descriptor_t make_task_gate( word segselector, byte atr);



#define _set_ds(valor) __asm__ __volatile__ ("movw %w0,%%ds\n\t" : : "r" (valor))
#define _set_es(valor) __asm__ __volatile__ ("movw %w0,%%es\n\t" : : "r" (valor))
#define _set_fs(valor) __asm__ __volatile__ ("movw %w0,%%fs\n\t" : : "r" (valor))
#define _set_gs(valor) __asm__ __volatile__ ("movw %w0,%%gs\n\t" : : "r" (valor))
#define _set_ss(valor) __asm__ __volatile__ ("movw %w0,%%ss\n\t" : : "r" (valor))
#define _set_esp(valor) __asm__ __volatile__ ("movl %0,%%esp\n\t" : : "r" (valor))

#define _lidt(valor) __asm__ __volatile__ ("lidt (%0)" : : "q" (valor));
#define ldtr(valor) __asm__ __volatile__ ("lldt %w0" : : "a" (valor));



