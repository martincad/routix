/*!  \addtogroup BootStrap
	\page init.c
	Módulo de arranque de Routix. Encargado de reubicar al sistema en memoria.	
*/

#include "../../include/system.h"


#define POSICION_LOADER		0xff800000	/* ubicacion del kernel.bin por parte del loader */



extern void kmain (void);	


extern unsigned char _data_size;
extern unsigned char _code_size;



void init(void)
{
	char *origen, *destino; 
	long i, largo;
	
	/* Copiar el kernel */

	destino = (char *) KERNEL_FCODE;
	origen = (char *) POSICION_LOADER;
	
	
	for(i=0 ; i < (long) &_code_size; i++)
		*(destino+i) = *(origen+i);
	

	__asm__ __volatile__ ("address_X: jmp (address_X - 0xff800000 + 5) ");
	
	
	/* Copiar Datos */

	destino = (char *) KERNEL_FDATA;
	origen = (char *) POSICION_LOADER + (long) &_code_size;

	largo= (long) &_data_size;
	
	for(i=0; i < largo ; i++)
		*(destino+i) = *(origen+i);

	/* Una vez copiado todo en su lugar, saltamos a funcion ppal*/
	kmain();

}








