/* sys_mem.c: llamadas al sistema del grupo SYS_MEM */

#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/errno.h"
#include "../../include/kalloc.h"
#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas

extern task_struct_t *actual;



// Vector de funciones de llamadas al sistema (grupo Timer)
int (*syscall_mem[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_malloc_page,
	(int (*) (void)) sys_free_page,
	(int (*) (void)) sys_free_mem
};

    

/* SYS_MALLOC_PAGE: Esta llamada tiene por función brindarle una pagina a un proceso de usuario, dandole la direccion
 *  virtual que le corresponde. Esta funcion se encarga también, de actualizar la task_Struct con los nuevos valores.
 */
void *sys_malloc_page(void)
{
    //Verificar que la tarea actual no sobrepase la cantidad máxima de páginas utilizadas
    word npages_usadas = actual->num_code + actual->num_data;

    if ( npages_usadas >= MAX_PAGINAS_POR_TAREA )
	return NULL;	//No más memoria para esta tarea

    //Debo devolver la dirección mapeada en el espacio virtual de la tarea. Desde la dir virtual TASK_DATA me desplazo
    //num_data paginas
    addr_t dir_virtual = TASK_DATA + (actual->num_data * PAGINA_SIZE);
    
    struct user_page *mem;

    //Posicionarse en el ultimo nodo
    for (mem=actual->mdata; mem->next ; mem=mem->next);
    
    //Pedir memoria al Memory Manager
    mem->next = umalloc_page (PAGINA_ALLOC,  dir_virtual);	
    
    //Si no hay disponible, nos vamos :-(
    if (!mem->next)
	return NULL;
    
    mem = mem->next;
    
    if (kmapmem( mem->dir , dir_virtual, actual->cr3_backup , PAGE_PRES|PAGE_USER|PAGE_RW) != OK) {
	kprintf("\nTEMP: KmapMem Retorno -1");
    }
	    
    actual->num_data++;

    return (void *) dir_virtual;
}	



/* Libera una pagina del User Space NO IMPLEMENTADA */
int sys_free_page(void *dir)
{
    kprintf("SYS_FREE_PAGE\n");
    while(1);
}


// Devuelve la cantidad de paginas libres (utilizada para propositos de depuración)
dword sys_free_mem (void)
{
    return kmem_free();
}





