/* int.c */

#include "../include/paging.h"
#include "../include/segm.h"
#include "../include/8259.h"
#include "../include/debug.h"
#include "../include/kalloc.h"
#include "../include/stdio.h"

#include "../include/fat.h"

#ifndef __SYSTEM
#include "../include/system.h"
#endif

#include "../include/event.h"


//extern byte floppy_continuar;
extern void intDefaultHandler(void);
extern void intTeclado(void);
extern void switch_to_kernel_mode(void);
extern void intProximaTarea(void);
extern void excepcion0(void);
extern void excepcion1(void);
extern void excepcion2(void);
extern void excepcion3(void);
extern void excepcion4(void);
extern void excepcion5(void);
extern void excepcion6(void);
extern void excepcion7(void);
extern void excepcion8(void);
extern void excepcion9(void);
extern void excepcion10(void);
extern void excepcion11(void);
extern void excepcion12(void);
extern void excepcion13(void);
extern void excepcion14(void);
extern void excepcion15(void);
extern void excepcion16(void);
extern void excepcion17(void);
extern void excepcion18(void);
extern void excepcion19(void);

pd_t *directorio = (pd_t *) POSICION_DIR_PAGINAS;

void DefaultHandler(void)
{
//    putchar('I');
//    putchar('\n');
    //continuar=1;
    endOfInterrupt();
}


void ExcepcionHandler(int numero,dword err1, dword err2, dword err3)
{
 char *excepciones[] = { "Divide error",
	 		 "Debug",
 			 "NMI Interrupt",
			 "Breakpoing",
			 "Overflow",
			 "Bound range exceeded",
			 "Invalid opcode",
			 "Device not available",
			 "Double fault",
			 "Coprocessor segment overrun",
			 "Invalid TSS",
			 "Segment not present",
			 "Stack Fault",
			 "General Protection",
			 "Page Fault",
			 "",
			 "x87 FPU floating point error",
			 "Alignment check",
			 "Machine check",
			 "SIMD floating point" };

 kprintf("\n EXCEPCION %d - %s\n",numero,excepciones[numero]);
 kprintf(" Error 1: 0x%x\n",err1);
 kprintf(" Error 2: 0x%x\n",err2);
 kprintf(" Error 3: 0x%x\n",err3);
 kprintf("Deteniendo el sistema.");

 while(1);
}

/*
#define TECLADO_BUFFER_SIZE 10
#define TECLADO_PORT 0x60

unsigned char bufferTeclado[TECLADO_BUFFER_SIZE];
unsigned char *ptrbufferTeclado=bufferTeclado;

unsigned char Keyboard_Controller[] =
{
   0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 225, 39, '\b',
   0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 129, '+', '\n',
   0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 148, 132, '^', 0, '#',
   'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
   0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, '<', 0, 0
};


void Teclado(void) {

  char valor;

  endOfInterrupt();

  valor=inportb(TECLADO_PORT);

  if ( ptrbufferTeclado >= bufferTeclado+TECLADO_BUFFER_SIZE ) {
   // Buffer lleno
  }
  
  else if ( valor & 0x80 ) {
    // Es un break code
    // por ahora no hacemos nada
    return;
  }
  
  else {
   *ptrbufferTeclado++ = Keyboard_Controller[valor];
  }

  // Actualizamos los eventos de TECLADO
  actualizar_eventos( TECLADO );

}
*/
/* Esto va ahora en el archivo drivers/floppy.c
void Floppy (void)
{
    continuar=1;
    endOfInterrupt();
}	
*/


void inicializarInterrupciones( void )
{
pt_t *ptIDT;
descriptor_t *paginaIDT;
idtr_t *idtr_so;
page_index_t pi;



paginaIDT = (descriptor_t *) kmalloc_page();
ptIDT = (pt_t *) kmalloc_page();

//directorio->entry[513] = make_pde(1,1,0,0,0,0,0,0,(char *) ptIDT);

pi = get_page_index( (addr_t) KERNEL_INTERRUPT );

kprintf("Entrada de pagina de interrupciones: %x\n", ptIDT);
kprintf("Interrupciones: %x (real) - %x (virtual)\n", paginaIDT, KERNEL_INTERRUPT );
kprintf("Indice de directorio y tabla: %d - %d\n", pi.dir_index, pi.tabla_index);

directorio->entry[ pi.dir_index ] = make_pde((addr_t) ptIDT, PAGE_RW | PAGE_PRES );
ptIDT->entry[ pi.tabla_index ] = make_pte((addr_t) paginaIDT, PAGE_PRES);

int i;
descriptor_t *temp;


for (i=0,temp = paginaIDT; i<256; i++,temp++)
	*temp = make_int_gate( (word) DESC_CODE, (dword) intDefaultHandler, (byte) (PRESENTE | DPL_0)  );

// Manejo de excepciones
#define excepcion(numero,nombre) *(paginaIDT+numero) = make_int_gate( (word) DESC_CODE, (dword) nombre, (byte) (PRESENTE | DPL_0)  );

excepcion(0,excepcion0);
excepcion(1,excepcion1);
excepcion(2,excepcion2);
excepcion(3,excepcion3);
excepcion(4,excepcion4);
excepcion(5,excepcion5);
excepcion(6,excepcion6);
excepcion(7,excepcion7);
excepcion(8,excepcion8);
excepcion(9,excepcion9);
excepcion(10,excepcion10);
excepcion(11,excepcion11);
excepcion(12,excepcion12);
excepcion(13,excepcion13);
excepcion(14,excepcion14);
excepcion(15,excepcion15);
excepcion(16,excepcion16);
excepcion(17,excepcion17);
excepcion(18,excepcion18);
excepcion(19,excepcion19);

//*(paginaIDT+0x20) = make_task_gate( (word) DESC_TSS0, (byte) (PRESENTE | DPL_0) );
*(paginaIDT+0x20) = make_int_gate( (word) DESC_CODE, (dword) switch_to_kernel_mode, (byte) (PRESENTE | DPL_0)  );
*(paginaIDT+0x21) = make_int_gate( (word) DESC_CODE, (dword) intTeclado, (byte) (PRESENTE | DPL_0)  );

extern void intFloppy(void);
*(paginaIDT+0x26) = make_int_gate( (word) DESC_CODE, (dword) intFloppy, (byte) (PRESENTE | DPL_0)  );

// Entrada desde el modo usuario
extern void intSysCall(void);
*(paginaIDT+0x50) = make_trap_gate( (word) DESC_CODE, (dword) intSysCall, (byte) (PRESENTE | DPL_3)  );

// Switch de proceso, utilizado cuando un proceso pasa al estado TASK_STOPPED
*(paginaIDT+0x51) = make_int_gate( (word) DESC_CODE, (dword) intProximaTarea, (byte) (PRESENTE | DPL_0)  );

idtr_so = (idtr_t *) (paginaIDT+256);

idtr_so->limite = 256*8 - 1;
idtr_so->base   = (dword) paginaIDT;



_lidt(idtr_so);

}






