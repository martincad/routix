#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"

#include "../../include/task.h"
#include "../../include/fat.h"


void kmain (void)
{
	addr_t mem;

	/* Deshabilitamos todas las interrupciones */
	mascarasPics(0xff,0xff);

	init_MM();

	/* Seguimos la ejecución en la dirección virtual */
	__asm__("aca: jmp aca+5+0xc0000000");

	/* Acomodamos el stack pointer a su valor virtual
	 * 0x200000    es el TOP FISICO  del STACK
	 * 0xd8000000  es el TOP VIRTUAL del STACK */
	__asm__ __volatile__ ("movl $0x200000, %%eax\n\t" \
			      "subl %%esp, %%eax\n\t"     \
			      "movl %%eax, %%esp\n\t"      \
			      "movl $0xd8000000,%%eax\n\t" \
		       	      "subl %%esp, %%eax\n\t"
			      "movl %%eax, %%esp" : : : "eax" );


	clrscr();
	puts("Inicializando kernel....\n");

	puts("Inicializando Controladores Programables de Interrupciones...\n");
	inicializarPics(0x20,0x28);
		
	puts("Kmalloc Inicializando...\n");
	kprintf("Memoria fisica %d Megabytes\n",memoria);

	inicializacion_kmalloc(memoria, KERNEL_END );
	puts("Kmalloc Inicializacion completa\n");

	/* Mapea linealmente el resto de la memoria fisica (a partir de los 4Mb en adelante) */
	init_all_memory(memoria);


	puts("Inicializando interrupciones\n");
	inicializarInterrupciones();

	puts("Inicializando 8254\n");
	init_8254();

	puts("Inicializando Rejoj\n");
	init_time();

	// Inicialización del scheduler
	puts("Inicializando el scheduler\n");
	start_scheduler();

	extern dev_fat_t dev_fat[1];		//Estructura para dispositivos con fs FAT
        dev_fat[0].boot_leido = FALSE;		
	dev_fat[0].fat_levantada = FALSE;
	floppy_cache_init();
	 
	// Habilitamos solo teclado, timertick y floppy

	kprintf("Tamaño del task_Struct: %d\n", sizeof(task_struct_t));

	enable_irq(0);
	enable_irq(1);
	enable_irq(6);

	entrada_de_inicio();

}

