/*!  \addtogroup MemoryManager
	\page kalloc.c
	Administra la memoria del kernel.
*/


#ifndef __SYSTEM
#include "../../include/system.h"
#endif
#include "../../include/stdio.h"
#include "../../include/kalloc.h"
#include "../../include/task.h"
#include "../../include/errno.h"
#include "../../include/debug.h"

#include "../../include/misc.h"

addr_t *_inicio,*_fin,*_sp;


/*
 *
 *
 *	_inicio ->	|---------------|
 *			|		|
 *			|		|
 *			|		|
 *	_sp     ->   	|xxxxxxxxxxxxxxx|
 *			|		|
 *			|		|
 *	_fin	->	|---------------|
 *
 * */



int kmalloc_page_start (addr_t *inicio, addr_t *fin)
{
_inicio = inicio;
_fin = fin;
_sp = fin;
return OK;
}

int kfree_page(addr_t direccion)
{
if ( _sp < _inicio ) { return NULL; }
if (getvar("mmdebug")==1)
	kprintf("KFREE: 0x%x\n", direccion);
*_sp-- = direccion;

return 1;
}


addr_t kmalloc_page()
{
    return get_free_page();
}

addr_t get_free_page()
{
		
addr_t direccion;

if ( _sp > _fin ) { return NULL; }

_sp++;
direccion = *_sp;

if (getvar("mmdebug")==1)
	kprintf("KMALLOC: _inicio=0x%x _fin=0x%x _sp=0x%x - valor: %x\n",_inicio,_fin,_sp,direccion);


return direccion;
}


void print_free_pages(word cant)
{
    word i;
    for(i=0 ; i<cant ; i++)
	kprintf("Direccion: %x\n", *(_sp+i));
}	



/* Bien, supongamos ahora que determinamos la cantidad de memoria del sistema
 * en función de esa cantidad armamos el área de stack necesaria para pushear
 * bloques de 4Kb (memoria física libre)
 */
void inicializacion_kmalloc(int memoria_fisica, int memoria_kernel)
{

int memoria_libre;
int mm_stack_size,paginas;
int pagina,pag;

memoria_libre = memoria_fisica - memoria_kernel / 0x100000;

kprintf("Memoria: \tInstalada %d Mb\tKernel %d Mb\tLibre %d Mb\n",memoria_fisica, memoria_kernel/0x100000,memoria_libre);

paginas = 1024*1024*memoria_libre / 4096;
mm_stack_size = 4 * paginas;

kprintf("El tamanio del stack de kmalloc_page para %d paginas es %d bytes - ",paginas,mm_stack_size);
kprintf("Inicio: 0x%x - Fin: 0x%x\n",memoria_kernel, memoria_kernel + mm_stack_size);


kmalloc_page_start( (dword *) memoria_kernel, (dword *) ( memoria_kernel + mm_stack_size ) );

/* Determinamos la primer pagina de memoria libre */
pagina = memoria_kernel + mm_stack_size;

if ( pagina & 0xfff ) {
	pagina += 0x1000 - ( pagina & 0xfff );
}
else {
	pagina = 0x1000 + pagina;
}


for( pag=((memoria_fisica*1024*1024)&0xfffff000)-4096 ; pag >= pagina ; pag -= 0x1000 ) {
	if ( kfree_page( (dword) pag ) == NULL ) { puts("error\n"); }
}

	
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Devuelve la cantidad de paginas disponibles del MM
//////////////////////////////////////////////////////////////////////////////////////////////
dword kmem_free (void)
{
    return  (_fin - _sp);
}	




// Pide una pagina y completa una estructura user_page (SOLO PARA PROCESOS DE USUARIO)
struct user_page *umalloc_page ( word flags, addr_t vdir)
{
    addr_t aux = get_free_page();
    if (!aux) {
		actual->err_no = ENOMEM;
		return NULL;
    }
    struct user_page *mem = (struct user_page *) malloc (sizeof(struct user_page));
    if (!mem) {
		kfree_page (aux);
		actual->err_no = ENOMEM;
		return NULL;
    }
    mem->dir = aux;
    mem->vdir = vdir;
    mem->flags = flags;
    mem->count = 1;
    mem->next = NULL;    
    return mem;    
}

//Libera una estructura descriptora de pagina de tarea, incluyendo al nodo que contiene su direccion
struct user_page *ufree_page (struct user_page *aux)
{
	aux->count--;
	if (aux->count > 0)
		return aux;

	if (aux->dir)
		kfree_page(aux->dir);

	free(aux);
	return NULL;
}

