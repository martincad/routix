/*!  \addtogroup MemoryManager
	\page mm.c
	Core de la administración de memoria. Inicializa la memoria y al mismo Memory Manager.
	No es utilizado una vez que el sistema ha sido inicializado.
*/

#include "system.h"
#include "paging.h"
#include "segm.h"
#include "kalloc.h"
#include "task.h"
#include "debug.h"

addr_t KERNEL_PDT = POSICION_DIR_PAGINAS;

void init_MM_base(void);
dword contar_memoria (void);
dword memoria;


/* Esta funcion inicializa 3 Tablas de paginas:
 * 1. Apunta de 0-4MB fisicos (donde se haya el kernel y los datos)
 * 2. Apuntan a los 3GB (direccion virtual del kernel)
 * 3. Apunta a 3GB + 128MB (direccion virtual de kernel data)
*/
void init_MM_base()
{
	page_index_t indice;
	pd_t *dir;
	pt_t *tabla;
	word i;
	gdtr_t *gdtr;
	descriptor_t *gdt;

	dir= (pd_t *) POSICION_DIR_PAGINAS;

	/* Mapear linealmente en Tabla 1 de 0-4MB (posicion fisica del kernel */
	indice=get_page_index( 0 );
	dir->entry[ indice.dir_index ]=make_pde (POSICION_TABLA_1, PAGE_PRES | PAGE_SUPER | PAGE_RW);
	tabla = (pt_t *) (dir->entry[ indice.dir_index ] & 0xfffff000);
	for(i=0 ; i < PAGINAS_POR_TABLA ; i++)
		tabla->entry[i]= make_pte( i * PAGINA_SIZE, PAGE_PRES | PAGE_SUPER | PAGE_RW ) ;


	/* Mapear El codigo de kernel en 3GB (0xC0000000) */
	indice=get_page_index(KERNEL_CODE);
	dir->entry[ indice.dir_index ]=make_pde (POSICION_TABLA_KCODE, PAGE_PRES | PAGE_SUPER | PAGE_RW);
	tabla = (pt_t *) (dir->entry[ indice.dir_index ] & 0xfffff000);
	for(i=0 ; i < PAGINAS_POR_TABLA ; i++)
		tabla->entry[i]= make_pte( KERNEL_FCODE + (i * PAGINA_SIZE), PAGE_PRES | PAGE_SUPER | PAGE_RW ) ;

	
	/* Mapear Los datos de kernel en 3GB + 128MB (0xC8000000) */
	indice=get_page_index(KERNEL_DATA);
	dir->entry[ indice.dir_index ]=make_pde (POSICION_TABLA_KDATA, PAGE_PRES | PAGE_SUPER | PAGE_RW);
	tabla = (pt_t *) (dir->entry[ indice.dir_index ] & 0xfffff000);
	for(i=0 ; i < PAGINAS_POR_TABLA ; i++)
		tabla->entry[i]= make_pte( KERNEL_FDATA + (i * PAGINA_SIZE), PAGE_PRES | PAGE_SUPER | PAGE_RW ) ;

	/* Mapear el stack de kernel en 3GB + 128MB + 128Mb + 128MB - 128Kb
	 * mapeamos desde el TOP 128Kb hacia arriba (32 páginas) */

	
	indice=get_page_index(KERNEL_STACK_TOP - KERNEL_STACK_SIZE);
	dir->entry[ indice.dir_index ]=make_pde (POSICION_TABLA_KSTACK, PAGE_PRES | PAGE_SUPER | PAGE_RW);
	tabla = (pt_t *) (dir->entry[ indice.dir_index ] & 0xfffff000);
	for(i=0 ; i < KERNEL_STACK_SIZE / PAGINA_SIZE; i++)
	 tabla->entry[i+indice.tabla_index]= make_pte( KERNEL_FSTACK + (i * PAGINA_SIZE), PAGE_PRES | PAGE_SUPER | PAGE_RW );


	/* Cargamos ahora la gdt */
	gdtr = (gdtr_t *) POSICION_TABLA_GDT;
	gdt  = (descriptor_t *) POSICION_TABLA_GDT;

	*gdt++ = make_descriptor( 0, 0, 0, 0);
	*gdt++ = make_descriptor( 0, 0xfffff, PRESENTE | DPL_0 | GENERAL | CODIGO, GRANULARIDAD | DB);
	*gdt++ = make_descriptor( 0, 0xfffff, PRESENTE | DPL_0 | GENERAL | DATA | WRITE, GRANULARIDAD | DB);
	*gdt++ = make_descriptor( 0, 0xfffff, PRESENTE | DPL_3 | GENERAL | CODIGO, GRANULARIDAD | DB);
	*gdt++ = make_descriptor( 0, 0xfffff, PRESENTE | DPL_3 | GENERAL | DATA | WRITE, GRANULARIDAD | DB);
	*gdt++ = make_descriptor( (dword) &tss, sizeof(tss_t), PRESENTE | DPL_0 | SISTEMA | TSS_AVAILABLE, DB);

	gdtr->limite =(dword) gdt - (dword) POSICION_TABLA_GDT - 1;
	gdtr->base   =(dword) POSICION_TABLA_GDT;


	lgdt( gdtr );

	_set_ds(DESC_DATA);
	_set_es(DESC_DATA);
	_set_fs(DESC_DATA);
	_set_gs(DESC_DATA);

	ltr(DESC_TSS0);
	
	
}	


void init_MM (void)
{
	dword memoria_total=0;

	page_off();	/* Deshabililitar paginacion */

	memoria_total=contar_memoria();	/* Contar memoria total del sistema */

	/* Habilitar 1 Tabla de 0-4MB, otra con Kernel Code (0xC0000000) y otra con Kernel Data (0xC8000000) */
	init_MM_base();

	load_cr3( POSICION_DIR_PAGINAS );
	page_on();

	memoria = memoria_total;
}


#define STEP	0x10000

dword contar_memoria (void)
{	
 dword *dir = (dword *) 0x200000;
 dword data = 0x12345678;
 dword prev;

 prev = *dir;
 *dir = data;

 while ( *dir == data) {
  *dir = prev;
  dir += STEP;
  prev = *dir;
  *dir = data;
 }
		 
 return ( (dword) dir / 0x100000 );
}
 


void init_all_memory( dword memoria ) {

	int i,j;
	page_index_t indice;
	pt_t *pt;

	pd_t *dir= (pd_t *) POSICION_DIR_PAGINAS;
	
	/* Mapear linealmente en el resto de la memoria
	 * desde los 4Mb en adelante */
	for ( i=1; i < memoria/4 ; i++) {

#if __VMDEBUG
		kprintf("PDE Numero %d (0x%x)\n",i,i*PAGINA_SIZE*1024);
#endif
		indice = get_page_index(i*PAGINA_SIZE*1024);
		pt = (pt_t *) kmalloc_page();

		dir->entry[ indice.dir_index ] = make_pde((addr_t)pt, PAGE_PRES | PAGE_SUPER | PAGE_RW);

		for ( j=0; j < PAGINAS_POR_TABLA; j++) {
			pt->entry[j] = make_pte( i*PAGINA_SIZE*1024 + j*PAGINA_SIZE, PAGE_PRES | PAGE_SUPER | PAGE_RW);
		}

	}

}

