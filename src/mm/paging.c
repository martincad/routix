/*!  \addtogroup MemoryManager
	\page paging.c
	Administración de la memoria paginada. Esta compuesto por funciones dedicadas a la traducción de direcciones físicas
	a lineales y viceversa. Prepara los directorios y tablas de páginas, para accesos lógicos.
*/

#include "system.h"
#include "paging.h"
#include "debug.h"
#include "stdio.h"
#include "kalloc.h"



/* Devuelve en la estructura la posicion dentro del DIR de paginas, y dentro de la Tabla */
page_index_t get_page_index (addr_t p)
{	
	page_index_t indice;

	indice.dir_index = p / ( PAGINA_SIZE * TABLAS_POR_DIR);
	indice.tabla_index = (p % ( PAGINA_SIZE * TABLAS_POR_DIR)) / PAGINA_SIZE;
	return indice;
}


//! Convierte una direccion logica respecto a cierto CR3 a una direccion de memoria fisica
void *convertir_direccion ( void *dir_logica, addr_t cr3 )
{
    void *dir_fisica;
    
    page_index_t indice;

    // Obtener los indices del directorio y de la tabla de paginas para la direccion logica
    indice = get_page_index ( (addr_t) dir_logica );

    pd_t *dir= (pd_t *) cr3;
    // Obtener la direccion de la tabla correspondiente y enmascarar los 12 bits de atributos
    pt_t *tabla = (pt_t *) (dir->entry[indice.dir_index] & 0xfffff000);
    // Obtener la direccion fisica y enmascarar los 12 bits de atributos
    dir_fisica = (void *) (tabla->entry[indice.tabla_index] & 0xfffff000);

    // Sumarle a la pagina obtenida de la tabla el offset de la direccion logica
    dir_fisica = dir_fisica + ((dword) dir_logica & 0xfff);

    return dir_fisica;
}	


/* Crea una entrada de directorio (tanto para directorio como para tabla, ya que son iguales en IA32) */
pde_t make_pde (addr_t direccion, word atributos)
{	
	pde_t entrada;
	entrada= atributos & 0xfff;
	entrada= entrada | direccion;

	return entrada;
}

/* Crea una entrada de Pagina (tanto para directorio como para tabla, ya que son iguales en IA32) */
pte_t make_pte (addr_t direccion, word atributos)
{	
	pte_t entrada;
	entrada= atributos & 0xfff;
	entrada= entrada | direccion;

	return entrada;
}



/* Mapea una dirección física en una logica */
/* ej.:
 * 	mem = kmalloc_page();
 *	if ( ! kmapmem( fisica, mem, 0xe0000000, PAGE_PRES ) )
 *		kfree_page(mem);
 */
#define	ERR_DIR_BUSY	-1
#define	ERR_NO_ALIGN	-2

#define	ERR_DIR_EMPTY	-5

byte DEBUG_PAGINACION=TRUE;

int kmapmem ( addr_t fisica, addr_t logica, addr_t directorio, word atributo)
{
	page_index_t indice;
	indice = get_page_index ( logica );
	pd_t *dir;
	pt_t *tabla;
	word i;
	
	atributo = atributo & 0xfff;

	if ( (fisica & 0xfff) || (logica & 0xfff) ) {
	    if ( getvar("pagedebug")==1 )
			kprintf("Debug Mode - Fis2log: las direcciones deben estar alineadas a 4KB\n");
	    return ERR_NO_ALIGN;

	}

    if ( getvar("pagedebug")==1 ) 	{
		 kprintf("Kmapmem: Mapeando 0x%x en 0x%x\n", fisica, logica);
		 kprintf("Kmapmem: Indice DIR: %d  Indice Tabla: %d\n", indice.dir_index, indice.tabla_index);
	}

	dir= (pd_t *) directorio;

	if (dir->entry[indice.dir_index] ==  0 ) 	{
	    if ( getvar("pagedebug")==1 )
		 	kprintf("Kmapmem: Debe alocarse una nueva tabla de paginas\n");
	
		dir->entry[indice.dir_index]= (pde_t) make_pde ( kmalloc_page(), PAGE_PRES | PAGE_SUPER | PAGE_RW);
		tabla = (pt_t *) (dir->entry[indice.dir_index] & 0xfffff000);
		/* Poner todas las pte en 0 */
		for (i=0; i < PAGINAS_POR_TABLA ; i++)
			tabla->entry[i]=(pte_t) make_pte (0, 0);
	}
	
	else    {	// Si la entrada existe, esa pagina puede ya estar asignada

		tabla = (pt_t *) (dir->entry[indice.dir_index] & 0xfffff000);
		if (tabla->entry[indice.tabla_index]) {
		    if ( getvar("pagedebug")==1 )
    			kprintf("Kmapmem: Pagina ya en uso\n");
			return ERR_DIR_BUSY;
		}
	}

	tabla->entry[indice.tabla_index]= (pte_t) make_pte ( fisica , atributo);

    return OK;
}


int kunmapmem (addr_t logica, addr_t directorio)
{
    if ( logica & 0xfff)  {
	    if ( getvar("pagedebug")==1 )
		    kprintf("KunmapMem: la direccion debe estar alineada a 4KB\n");
		return ERR_NO_ALIGN;
    }

    
    page_index_t indice;
    indice = get_page_index ( logica );

    pd_t *dir = (pd_t *) directorio;

    pt_t *tabla = (pt_t *) (dir->entry[indice.dir_index] & 0xfffff000);

    
    if ( !tabla ) {	//Si la entrada correspondiente a "logica" esta vacia
    	if ( getvar("pagedebug")==1 )
	   		kprintf("Debug Mode - KunmapMem: la tabla esta vacia\n");
	    return ERR_DIR_EMPTY;
    }

    //Liberar la entrada correspondiente
    tabla->entry[indice.tabla_index] = 0;


    //Ahora verifico si todas las entradas de la tabla estan vacias. Si es asi la libero. Con que haya una
    //ocupada, no puedo hacerlo
    word i;
    for ( i=0 ; i < PAGINAS_POR_TABLA ; i++)
//	if ( tabla->entry[indice.tabla_index] )
		if ( tabla->entry[i] )
			return OK;

	if (getvar("pagedebug")==1)
		kprintf("Kunmapmem: liberando tabla en: 0x%x\n", dir->entry[indice.dir_index] & 0xfffff000);
    kfree_page( (addr_t) (dir->entry[indice.dir_index] & 0xfffff000) );    
    dir->entry[indice.dir_index] = 0;
    return OK;

}	


void *make_pdt (void)
{
    addr_t nuevo_directorio = kmalloc_page();
    if ( nuevo_directorio == 0) {
		return NULL;
    }

//	memset(nuevo_directorio, 0, PAGINA_SIZE);
    copy_page( (void *) nuevo_directorio, (void *) KERNEL_PDT);
//kprintf("TEMP: make_pdt: Retornando directorio: 0x%x\n", nuevo_directorio);
	return (void *) nuevo_directorio;
}	

// Copia una pagina completa, moviendo de a 4 bytes (movsl) una cantidad de PAGINA_SIZE / sizeof(dword) (1024 veces
// si PAGINA_SIZE=4096 y el dword es de 4 bytes
void *copy_page( void *dest, const void *src)
{
    __asm__("push %%ecx; push %%edi ; push %%esi ;cld ; rep ; movsl ; pop %%esi ; pop %%edi ; pop %%ecx" \
		    :  : "c" ((word) (PAGINA_SIZE / (sizeof(dword)))), "S" (src), "D" (dest));
    
    return dest;
}

