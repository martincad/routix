#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"

#include "../../include/task.h"
#include "../../include/fat.h"
#include "../../include/syscalls.h"

// Puntero a pagina de wrappers de usuario
addr_t *exit_addr;

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

	kprintf("Task Struct Size: %d\n", sizeof(task_struct_t));

	enable_irq(0);
	enable_irq(1);
	enable_irq(6);
	
	// Pedimos una pagina, la cual pertenecera al Kernel pero tendra privilegios de usuario (solo lectura) donde se ubicaran
	// algunos wrappers, como ser la llamada a la System call EXIT
	exit_addr = (addr_t *) kmalloc_page();

	char *ptr_exit = (char *) exit_addr;
	
	// Ubicamos en el wrapper el codigo de libreria routstd de llamada EXIT:
	*ptr_exit = 0xb8; 	// Codigo de operacion: "mov eax, "
	*(unsigned long *)(ptr_exit+1) = SYS_PROCESS | SYS_EXIT; 
	*(ptr_exit+5) = 0xbb; // "mov ebx, "
	*(unsigned long *)(ptr_exit+6) = 0; // parametro que recibe la función EXIT... 0 en este caso
	*(ptr_exit+10) = 0xcd;	// int
	*(ptr_exit+11) = 0x50;	// 0x50

	kprintf("Ubicacion del Wrappeo de Exit: 0x%x\n", ptr_exit);

	entrada_de_inicio();

}

