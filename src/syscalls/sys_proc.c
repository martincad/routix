/* sys_proc.c: llamadas al sistema del grupo PROCESS */

#include "../../include/system.h"
#include "../../include/paging.h"
#include "../../include/segm.h"
#include "../../include/debug.h"
#include "../../include/syscalls.h"
#include "../../include/elf.h"
#include "../../include/file.h"
#include "../../include/errno.h"
#include "../../include/timer.h"
#include "../../include/kalloc.h"
#ifndef __TASK
#include "../../include/task.h"
#endif


//Nuevas

extern task_struct_t *actual;



// Vector de funciones de llamadas al sistema (grupo Process)
int (*syscall_process[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_exec,	
	(int (*) (void)) sys_void,	
	(int (*) (void)) sys_fork,	
	(int (*) (void)) sys_perror,	
	(int (*) (void)) sys_renice,	
	(int (*) (void)) sys_get_pid,
	(int (*) (void)) sys_get_ppid,
	(int (*) (void)) sys_exit,
	(int (*) (void)) sys_show
			
};

   

int sys_void (void)
{
    kprintf("\nLlamada a SYS Void\n");
    return OK;
    
}	

// Estructuras utilizadas por fork()
// Registros que se pusheand al stack de modo kernel al entrar via int 0x50
struct int_regs
{
    dword ebx;
    dword ecx;
    dword edx;
    dword edi;
    dword esi;
    dword ebp;
    dword eip;
    dword cs;
    dword eflags;
    dword esp;
    dword ss;
};
// Esta estructura posee todos los registros que se popean del stack de modo kernel antes de ejecutar una tarea
struct int_regs_ext
{
    dword gs;
    dword fs;
    dword es;
    dword ds;
    dword eax;
    dword ebx;
    dword ecx;
    dword edx;
    dword edi;
    dword esi;
    dword ebp;
    dword eip;
    dword cs;
    dword eflags;
    dword esp;
    dword ss;
};


int sys_fork (void)
{
    struct task_struct_t *new_task;
    // Punteros al contexto de la tarea (en el stack de modo kernel)
    struct int_regs *new, *current;
    struct int_regs_ext *new_ext;
    word i;

    // init_new_task devuelve la tarea creada como TASK_STOPPED, la mantendremos así hasta el final    
    new_task = init_new_task (0,0,0,0,0, actual->descripcion, actual->prioridad);
    
    current = (struct int_regs *) ((actual->esp0 & 0xfffff000) + PAGINA_SIZE - sizeof(struct int_regs));
    new = (struct int_regs *) ((new_task->esp0 & 0xfffff000) + PAGINA_SIZE - sizeof(struct int_regs));

    new_ext = (struct int_regs_ext *) ((new_task->esp0 & 0xfffff000) + PAGINA_SIZE - sizeof(struct int_regs_ext));
    
    new_ext->ds = DESC_DATA_USUARIO;
    new_ext->es = DESC_DATA_USUARIO;
    new_ext->fs = DESC_DATA_USUARIO;
    new_ext->gs = DESC_DATA_USUARIO;
    
    new_task->esp0 = (new_task->esp0 & 0xfffff000) + PAGINA_SIZE - 64;
    
    // Copiar los contextos
    memcpy(new, current, sizeof (struct int_regs) );

    // Duplicar los indicadores de cantidad de paginas usadas por los segmentos
    new_task->num_code = actual->num_code;
    new_task->num_data = actual->num_data;
    new_task->num_stack = actual->num_stack;
   
    struct user_page *mem;
    mem = new_task->mcode = actual->mcode;
    
    //Mapear las direcciones del segmento de codigo en memoria virtual, e incrementar el contador de procesos (count)
    for ( i=0 ; i < new_task->num_code ; i++,mem=mem->next ) {
		if ( kmapmem( mem->dir , mem->vdir , new_task->cr3 ,PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ){
		    kprintf("Kmapmem TASK_TEXT error\n");
		    return -1;
		}
		mem->count++;
    }
    
    struct user_page *src = actual->mdata;
    struct user_page *dest = (struct user_page *) malloc (sizeof(struct user_page));

    addr_t addr_virtual_data;

    //Para datos y bss debo pedir memoria, copiar la del proceso actual en ella, y finalmente mapearla
    for( i=0  ;  i < new_task->num_data ;  i++) {
		dest->next = umalloc_page ( src->flags, src->vdir );
		if (!dest) { //liberar
		    actual->err_no = ENOMEM;
		    return -1;
		}
		if (i==0)	//Apuntar el mdata al comienzo de la lista
		    new_task->mdata = dest->next;

		dest = dest->next;
		copy_page ( (void *)dest->dir, (void *)src->dir);
		if ( kmapmem( dest->dir , dest->vdir , new_task->cr3 , PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ){
		    kprintf("Kmapmem TASK_DATA error\n");
		    return -1;
		}
		src = src->next;
    }

    ////////// STACK
    src = actual->mstack;
    dest = (struct user_page *) malloc (sizeof(struct user_page));

    for( i=0  ;  i < new_task->num_stack ;  i++) {
		dest->next = umalloc_page ( src->flags, src->vdir );
		if (!dest) { //liberar
		    actual->err_no = ENOMEM;
		    return -1;
		}
		if (i==0)	//Apuntar el mdata al comienzo de la lista
		    new_task->mstack = dest->next;

		dest = dest->next;
		copy_page ( (void *)dest->dir, (void *)src->dir);
		if ( kmapmem( dest->dir , dest->vdir , new_task->cr3 , PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ){
		    kprintf("Kmapmem TASK_DATA error\n");
		    return -1;
		}
		src = src->next;
    }

    // Poner en eax del hijo un 0, para que tome como retorno de fork 0
    new_ext->eax = 0;

    new_task->ppid = actual->pid;

    // Despertamos la nueva tarea, recordemos que init_new_task crea las tareas en estado TASK_STOPPED
    despertar_task( new_task );

    return (new_task->pid);
}


/* Llamada al sistema exec: carga un ejecutable de disco (COFF32), verifica que posea sus 3 secciones, y lo carga en 
 * memoria. Genera una nueva tarea, cierra el descriptor.
 */
int sys_exec (char *nombre)
{

    nombre = convertir_direccion( nombre , actual->cr3_backup);
 
    int fd;
    byte buff_aux[100];	    //WARNING, agota el stack
    word i;
    
    struct coff_header *p;
    struct coff_sections sec_text, sec_data, sec_bss;

    int bytes_leidos;
    
    fd = open(nombre);
    if (fd < 0) {
	actual->err_no = ENOENT;
        return -1;
    }
    
    // Leer cabecera basica del COFF32
    read(fd, buff_aux, sizeof(struct coff_header));
    p = (struct coff_header *) buff_aux;

    if (p->f_magic != COFF32_TYPE) {	    // Verificar numero magico
	actual->err_no = ENOEXEC;
	close(fd);
	return -1;
    }
    byte num_secciones;
    if ( (num_secciones = p->f_nscns)!= 3) {
		actual->err_no = ENOEXEC;
		close(fd);
		return -1;
    }
    // Pararse donde comienzan las estructuras de las secciones ( header + datos_opcionales)
    lseek (fd, sizeof(struct coff_header) + p->f_opthdr , SEEK_SET);

    word verificacion = 0;	// Flag usado para verificar la existencia de .TEXT, .DATA y .BSS

    struct coff_sections *q;
    //Levantar las 3 secciones que debe tener el ejecutable
    while (num_secciones--) {
		read(fd, buff_aux, sizeof(struct coff_sections));
		q = (struct coff_sections *) buff_aux;
    
	//Por ahora no puede ejecutarse tareas con mas de 1 PAGINA de codigo
	if (q->s_size >= PAGINA_SIZE) {
	    actual->err_no = EFBIG;
	    close(fd);
	    return -1;
	}		
	
	if ( q->s_flags == COFF32_TEXT ) {
	    memcpy ( &sec_text , q , sizeof(struct coff_sections) );
	    verificacion = verificacion | COFF32_TEXT;
	}
	else if ( q->s_flags == COFF32_DATA ) {
	    memcpy ( &sec_data , q , sizeof(struct coff_sections) );
	    verificacion = verificacion | COFF32_DATA;
	}
	else if ( q->s_flags == COFF32_BSS ) {
	    memcpy ( &sec_bss , q , sizeof(struct coff_sections) );
	    verificacion = verificacion | COFF32_BSS;
	}
	else {	    //No se puede identificar el tipo de seccion
	    close(fd);
	    actual->err_no = ENOEXEC;
	    return -1;
	}
    }
        	    
    //Verificar que posea una seccion de codigo, una de datos inicializados y sin inicializar
    if (verificacion != (COFF32_TEXT | COFF32_DATA | COFF32_BSS) ) {
    	close(fd);
		actual->err_no = ENOEXEC;
		return -1;
    }
    
    //Verificar que haya memoria suficiente    
    if (kmem_free() < ((actual->open_files[fd]->size / PAGINA_SIZE)+4) ) {
		actual->err_no = ENOMEM;
		close(fd);
		return -1;
    }

    // Aca voy a guardar la cantidad de paginas requeridas por cada segmento
    word paginas_texto, paginas_datos;

    paginas_texto = sec_text.s_size / PAGINA_SIZE;
    if ( sec_text.s_size % PAGINA_SIZE )
		paginas_texto++;
    
    //Tamaño en bytes del .DATA + .BSS
    int size_datos = sec_data.s_size + sec_bss.s_size;

    //Cantidad de paginas requeridas por .DATA + .BSS
    paginas_datos = size_datos / PAGINA_SIZE;
    if ( size_datos % PAGINA_SIZE )
		paginas_datos++;

    //Cantidad de paginas requeridas solo por .DATA
    int paginas_data = sec_data.s_size / PAGINA_SIZE;
    if ( sec_data.s_size % PAGINA_SIZE )
		paginas_data++;

    task_struct_t *new_task;
    struct user_page *mem;
    
    // Poner el nombre de archivo como descripcion
    char nuevo_nombre[13];
    tomar_nombre_tarea(nombre,nuevo_nombre);

    // init_new_task devuelve la tarea creada como TASK_STOPPED, la mantendremos así hasta el final    
    new_task = init_new_task(DESC_CODE_USUARIO, DESC_DATA_USUARIO, TASK_TEXT, TASK_STACK + 4096 - 4, 0x202,\
		    nuevo_nombre, 10);
	
    if (!new_task) {	    //liberar
		return -1;
    }

    new_task->mstack = umalloc_page (PAGINA_STACK, TASK_STACK);
    if (!new_task->mstack)  //liberar
		return -1;

    if( kmapmem( new_task->mstack->dir , TASK_STACK, new_task->cr3 , PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ) {
		kprintf("TEMP 1:new_task->cr3: 0x%x \nStack_tarea: 0x%x\n", new_task->cr3, new_task->mstack->dir);	
        kprintf("Kmapmem TASK_STACK error\n");
        return -1;
    }

	// Direccion fisica de la ubicacion de wrappers para tareas 	
	extern addr_t *exit_addr;
	//mapeo los wrappers en una direccion virtual EXIT_TASK   
    if( kmapmem( exit_addr , EXIT_TASK , new_task->cr3 , PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ) {
        kprintf("Kmapmem EXIT_TASK error\n");
        return -1;
    }

	// Poner al fondo del stack la direccion de comienzo del codigo del wrapper exit
	unsigned long *qwe = (unsigned long *) (new_task->mstack->dir + 4096 - 4);
	*qwe = (unsigned long) EXIT_TASK;
	
    new_task->num_code = paginas_texto;
    new_task->num_data = paginas_datos;
    new_task->num_stack = 1;
    
    // Levantar el .TEXT
    lseek(fd, sec_text.s_scnptr,SEEK_SET);
    new_task->mcode = umalloc_page (PAGINA_CODE,TASK_TEXT);
    if (!new_task->mcode)    //liberar
		return -1;
    mem = new_task->mcode;
    bytes_leidos =  read( fd, (void *)mem->dir, PAGINA_SIZE);
	
    for( i=1 ;  i < paginas_texto ; i++) {
		mem->next = umalloc_page (PAGINA_CODE, TASK_TEXT + (i*PAGINA_SIZE) );
        bytes_leidos =  read(fd, (void *)mem->dir , PAGINA_SIZE);
		if (bytes_leidos < 0) {	    //error de algo.... liberar memoria
		    actual->err_no = EIO;
		    close(fd);
		    return -1;
		}
		mem = mem->next;
    }

    //Pedir memoria para los datos (.DATA + .BSS)
    new_task->mdata = umalloc_page (PAGINA_DATA, TASK_DATA);
    if (!new_task->mdata)   //liberar
		return -1;
    for ( mem=new_task->mdata,i=1 ; i<paginas_datos ; i++, mem=mem->next ) {
        mem->next = umalloc_page (PAGINA_DATA, TASK_DATA + (i*PAGINA_SIZE) );
		if (!mem->next)	    //liberar
		    return -1;
    }

    //Levantar .DATA
    lseek(fd, sec_data.s_scnptr,SEEK_SET);
    for( i=0, mem=new_task->mdata ;   i < paginas_data  ;  i++, mem=mem->next ) {
		bytes_leidos =  read(fd, (void *)mem->dir, PAGINA_SIZE);
		if (bytes_leidos < 0) {	    //error de algo.... liberar memoria
		    actual->err_no = EIO;
		    close(fd);
		    return -1;
		}
    };

    //Mapear codigo y datos a sus direcciones virtuales correspondientes (TASK_TEXT, TASK_DATA)
    for( mem=new_task->mcode,i=0 ;  mem  ;  mem=mem->next,i++ ) {
		if ( kmapmem( mem->dir , TASK_TEXT + (i*PAGINA_SIZE), new_task->cr3 ,PAGE_PRES|PAGE_SUPER|PAGE_RW)!=OK ) {
	    	kprintf("Kmapmem TASK_TEXT error\n");
		    return -1;
		}
    }

    for( mem=new_task->mdata,i=0 ; mem ;  mem=mem->next,i++ ) {
		if( kmapmem( mem->dir , TASK_DATA +(i*PAGINA_SIZE), new_task->cr3, PAGE_PRES|PAGE_SUPER|PAGE_RW)!=OK ) {
		    kprintf("Kmapmem TASK_DATA error\n");
		    return -1;
	    }
    }
    
    new_task->ppid = actual->pid;

    // Inicialización de senales
    new_task->senales.senales_pendientes = 0;

    close(fd);

    // Despertamos la nueva tarea, recordemos que init_new_task crea las tareas en estado TASK_STOPPED
    despertar_task( new_task );
    
    return OK;
}


void sys_perror (char *str)
{
    str = convertir_direccion( str , actual->cr3_backup);
    perror(str);
}


int sys_renice (word pid, word prioridad)
{
//    kprintf("TEMP SYS_RENICE. PID: %d\tPRI: %d\n", pid, prioridad);
    task_struct_t *tmp;
    if ( (tmp = encontrar_proceso_por_pid(pid))==NULL ) {
	actual->err_no = ESRCH;
	return -1;
    }
    // Esto es solo a modo de prueba ya que en routix por ahora el TASK_STOPPED cumple las veces de TASK_INTERRUMIPLE
    if (prioridad < 1)
	tmp->estado = TASK_STOPPED;
    tmp->prioridad = prioridad;    
    return OK;
}

inline pid_t sys_get_pid (void)
{
    return actual->pid;
}


inline pid_t sys_get_ppid (void)
{
    return actual->ppid;
}

void sys_exit (int valor)
{
	kprintf("SYS_EXIT: Implementacion a medio hacer de exit... recibio: %d\n", valor);
	dormir_task(actual);
	actual->estado = TASK_ZOMBIE;
		
	struct user_page *aux, *tmp;

	cli();
	
	for (aux=tmp=actual->mcode ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		kunmapmem(aux->vdir, actual->cr3_backup);
		ufree_page(aux);
		if (getvar("exitdebug")==1)
			kprintf("Libero Codigo\n");
	}
	for (aux=tmp=actual->mdata ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		kunmapmem(aux->vdir, actual->cr3_backup);
		ufree_page(aux);
		if (getvar("exitdebug")==1)
			kprintf("Libero Datos\n");
	}
	for (aux=tmp=actual->mstack ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		kunmapmem(aux->vdir, actual->cr3_backup);
		ufree_page(aux);
		if (getvar("exitdebug")==1)
			kprintf("Libero UserStack\n");
	}
	// Desmapear del directorio la pagina usada para los wrappers (de exit por ej)
	
	kunmapmem (TASK_TEXT - PAGINA_SIZE, actual->cr3_backup);

	// Antes de liberar a este muñequin, debo liberar a las tablas de pagina
	kfree_page (actual->cr3_backup);
//	kfree_page (actual);


	sti();
	_reschedule();

}

//Cantidad de veces que la función malloc llamo a morecore solicitandole una página
extern word morecores;
extern struct floppy_cache *header_floppy_cache;

void sys_show (int valor)
{
	int i;
	switch (valor) {
		case 1: 
			kprintf ("Cantidad de paginas asignadas via Morecores: %d\n", morecores);
			break;
		case 2:
			kprintf("Sectores en cache: ");
			struct floppy_cache *aux;
			for (aux = header_floppy_cache, i=0 ; aux ; aux=aux->next, i++)
				kprintf("%d ", aux->sector);
			kprintf("\nSizeof floppy_cache: %d\n", sizeof(struct floppy_cache));
			kprintf("Sectores cacheados: %d\tPaginas usadas: %d\n", i, sizeof(struct floppy_cache) * i / 4096 + 1);
			break;
		default:
			break;
	}
			
}

struct {
	char nombre[15];
	int	 valor;
} variables[15];
