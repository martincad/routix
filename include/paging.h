/* paging.h */

#ifndef __SYSTEM
 #include "system.h"
#endif




#define DIR_SIZE	4096
#define TABLA_SIZE	DIR_SIZE
#define PAGINA_SIZE	4096
#define TABLAS_POR_DIR  1024
#define PAGINAS_POR_TABLA 1024

#define POSICION_DIR_PAGINAS	( KERNEL_FSTACK_TOP - KERNEL_STACK_SIZE - DIR_SIZE)
#define POSICION_TABLA_1	( POSICION_DIR_PAGINAS - DIR_SIZE)
#define POSICION_TABLA_KCODE	( POSICION_TABLA_1 - TABLA_SIZE )
#define POSICION_TABLA_KDATA	( POSICION_TABLA_KCODE - TABLA_SIZE )
#define POSICION_TABLA_KSTACK	( POSICION_TABLA_KDATA - TABLA_SIZE )

//Tomamos 4Kb para la GDT 
#define POSICION_TABLA_GDT	( POSICION_TABLA_KSTACK - PAGINA_SIZE )


//! Atributos de Pagina
#define PAGE_RW		0X02
#define PAGE_PRES	0x01
#define PAGE_SUPER	1<<2
#define	PAGE_USER	1<<2
/*
#define PAGE_USER	1<<2
#define PAGE_SUPER	PAGE_USER
*/

//! Page Table entry
typedef dword pte_t;
//! Page Directory entry
typedef dword pde_t;

typedef struct pd_t {
	pde_t entry[1024];
} pd_t;

typedef struct pt_t {
	pte_t entry[1024];
} pt_t;

//! Estructura utilizada para indicar en que directorio y en que tabla se mapea una dirección lógica
typedef struct page_index_t
{
	word dir_index;
	word tabla_index;
} page_index_t;


//! Devuelve los indices del Directorio y tabla de paginas para una direccion lógica
page_index_t get_page_index (addr_t );


/* Carga CR3 */
#define load_cr3(x) __asm__ ("movl %0, %%cr3" : : "r" (x) )

/* Desactiva/Activa la paginacion */
#define page_off() __asm__ ("mov %cr0, %eax; and $0x7fffffff, %eax; mov %eax, %cr0")
#define page_on()  __asm__ ("mov %cr0, %eax; or $0x80000000, %eax; mov %eax, %cr0")

//! Genera un directorio de páginas para un proceso. Copia todo el mapeo del sistema contenido en el directorio del Kernel
void *make_pdt (void);

pde_t make_pde (addr_t direccion, word atributos);
pte_t make_pte (addr_t direccion, word atributos);

//! Copia una página entera (optimizado para velocidad)
void *copy_page (void *dest, const void *src);

//! Convierte una direccion logica respecto a cierto CR3 a una direccion de memoria fisica
void *convertir_direccion ( void *dir_logica, addr_t cr3 );

int kmapmem ( addr_t fisica, addr_t logica, addr_t directorio, word atributo);
int kunmapmem (addr_t logica, addr_t directorio);

