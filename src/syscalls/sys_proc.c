/*!  \addtogroup Syscalls
	\page sys_proc.c
	Contiene las llamadas al sistema relacionadas con procesos.
*/


#include "system.h"
#include "paging.h"
#include "segm.h"
#include "debug.h"
#include "syscalls.h"
#include "elf.h"
#include "file.h"
#include "error.h"
#include "timer.h"
#include "kalloc.h"
#include "stdio.h"
#include "string.h"
#ifndef __TASK
#include "task.h"
#endif


//! Puntero al task struct de la tarea que realizó la llamada al sistema
extern task_struct_t *actual;



//! Vector de funciones de llamadas al sistema (grupo Process)
int (*syscall_process[MAX_SYSCALLS]) (void) = {
	(int (*) (void)) sys_exec,	
	(int (*) (void)) sys_void,	
	(int (*) (void)) sys_fork,	
	(int (*) (void)) sys_perror,	
	(int (*) (void)) sys_renice,	
	(int (*) (void)) sys_get_pid,
	(int (*) (void)) sys_get_ppid,
	(int (*) (void)) sys_exit,
	(int (*) (void)) sys_show,
	(int (*) (void)) sys_waitpid
			
};

   
//! Sólo imprime un mensaje (utilizada tiempo atrás para propósitos de debug)
int sys_void (void)
{
    kprintf("\nLlamada a SYS Void\n");
    return OK;
    
}	

//! Estructuras utilizadas por fork()
//! Registros que se pushean al stack de modo kernel al entrar via int 0x50
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

//! Esta estructura posee todos los registros que se popean del stack de modo kernel antes de ejecutar una tarea
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

//! Implementación de fork (no hace falta hacer comentarios)
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

    new_task->pid = get_new_pid();
    new_task->ppid = actual->pid;

    // Despertamos la nueva tarea, recordemos que init_new_task crea las tareas en estado TASK_STOPPED
    despertar_task( new_task );

    return (new_task->pid);
}


/*! \brief Genera un nuevo proceso a partir de un ejecutable COFF32 (no pisa, momentaneamente, la copia del proceso llamante)
 *  \param path donde se encuentra el ejecutable
 *  \return -1 en caso de error.
 *  \todo Liberar recursos en caso de error
 *  \todo Que el proceso se ejecute con el PID y recursos de la tarea que lo llamó (POSIX 1003.1)
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
/*		if (q->s_size >= PAGINA_SIZE) {
		    actual->err_no = EFBIG;
		    close(fd);
		    return -1;
		}		
*/	
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
    if (kmem_free() < ((actual->open_files[fd]->size / PAGINA_SIZE)+8) ) {
		actual->err_no = ENOMEM;
		close(fd);
		return -1;
    }

    // Aca voy a guardar la cantidad de paginas requeridas por cada segmento
    word paginas_texto, paginas_datos;

    paginas_texto = sec_text.s_size / PAGINA_SIZE;
    if ( sec_text.s_size % PAGINA_SIZE ) {
		paginas_texto++;
	}
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
    if( kmapmem((addr_t)exit_addr , EXIT_TASK , new_task->cr3 , PAGE_PRES|PAGE_USER|PAGE_RW)!=OK ) {
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
		mem = mem->next;
        bytes_leidos =  read(fd, (void *)mem->dir , PAGINA_SIZE);
		if (bytes_leidos < 0) {	    //error de algo.... liberar memoria
		    actual->err_no = EIO;
		    close(fd);
		    return -1;
		}
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

// Si exec genera un nuevo proceso	
//    new_task->ppid = actual->pid;
//	new_task->pid = get_new_pid();

// Si exec pisa la copia del proceso llamante
	if (actual->pid==1) {				// Si el proceso es init, caso particular ya que init no tiene formato de tarea
	    new_task->ppid = actual->pid;
		new_task->pid = get_new_pid();
	}
	else {								// Si es cualquier otro proceso, el nuevo proceso debe heredar su pid y ppid
		new_task->ppid = actual->ppid;
	    new_task->pid = actual->pid;
	}

    // Inicialización de senales
    new_task->senales.senales_pendientes = 0;

    close(fd);

    // Despertamos la nueva tarea, recordemos que init_new_task crea las tareas en estado TASK_STOPPED
    despertar_task( new_task );

	// Recordemos que el proceso que ejecuto el exec debe terminar (excepto que sea el init)
	if (actual->pid!=1) {
		cli();
		sys_exit_mm();					// libero la memoria
		remover_task (actual);			// lo saco de la lista de tareas del scheduler
		kfree_page ((addr_t)actual);	// libero el task_struct
		sti();
		_reschedule();					
	}
	
    return OK;
}

//! \brief Imprime el mensaje relacionado a un errorn
void sys_perror (char *str)
{
    str = convertir_direccion( str , actual->cr3_backup);
    perror(str);
}

/*! \brief modifica la prioridad de una tarea
 *  \param pid del proceso
 *  \param nueva prioridad
 */
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
		dormir_task(tmp);
    tmp->prioridad = prioridad;    
    return OK;
}

//! \brief Devuelve el PID de la tarea actual
inline pid_t sys_get_pid (void)
{
    return actual->pid;
}

//! \brief Devuelve el PID de su padre
inline pid_t sys_get_ppid (void)
{
    return actual->ppid;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	Llamada al sistema Exit y funciones relacionadas
//
////////////////////////////////////////////////////////////////////////////////////////////////

//! Header de la lista de procesos "actualmente" zombies
struct zombie_queue zombie_header;


//! Cantidad de procesos en estado Zombie
int zombie_queue_len = 0;

// Funciones relacionadas con la llamada "exit"
int sys_exit_mm(void);
int sys_exit_notify(void);

/*! \brief Termina un proceso y libera los recursos utilizados
 *  \param valor retornado al padre (o a quién recoja la condición de salida)
 *
 *  \relates	sys_exit_notify
 */
/*
 *  \relates	sys_exit_notify
 *  \relates	sys_exit_mm
 *  \relates	sys_wait
 */
void sys_exit (int valor)
{
	cli();
	dormir_task(actual);
	actual->estado = TASK_ZOMBIE;
	actual->retorno = valor;	
	
	sys_exit_mm();
	sys_exit_notify();

	sti();
	_reschedule();
}

/*! \brief Libera todos los recursos de memoria utilizados por un proceso (excepto el task_struct)
 */
int sys_exit_mm(void)
{
	struct user_page *aux, *tmp;

	// Liberar las paginas de código
	for (aux=tmp=actual->mcode ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		// Desmapear la direccion fisica de la logica
		kunmapmem(aux->vdir, actual->cr3_backup);
		// Si Esta habilitada la variable de entorno "__exit", imprimir info
		if (getvar("__exit")==1)
			kprintf("Liberando Codigo dir: 0x%x\tvdir: 0x%x\n", aux->dir,aux->vdir);
		ufree_page(aux);
	}
	// Liberar de esta tarea las paginas de datos
	for (aux=tmp=actual->mdata ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		kunmapmem(aux->vdir, actual->cr3_backup);
		if (getvar("__exit")==1)
			kprintf("Liberando Datos dir: 0x%x\tvdir: 0x%x\n", aux->dir,aux->vdir);
		ufree_page(aux);
	}
	// Liberar de esta tarea las paginas de codig
	for (aux=tmp=actual->mstack ; aux && tmp ; aux=tmp) {
		tmp=aux->next;
		kunmapmem(aux->vdir, actual->cr3_backup);
		if (getvar("__exit")==1)
			kprintf("Liberando Stack dir: 0x%x\tvdir: 0x%x\n", aux->dir,aux->vdir);
		ufree_page(aux);
	}
	// Desmapear del directorio la pagina usada para los wrappers (de exit por ej)
	kunmapmem (TASK_TEXT - PAGINA_SIZE, actual->cr3_backup);

	// Libero el directorio de páginas utilizado por la tarea
	kfree_page (actual->cr3_backup);
	return 0;
}

/*! \brief notificación al sistema de la terminación de un proceso
 *  \nota  (uso interno de System Call exit)
*/	
int sys_exit_notify (void)
{
	struct zombie_queue *aux = &zombie_header;
	
	// Voy hacia el ultimo nodo de la lista
	for (aux=&zombie_header ; aux->next!=NULL ; aux=aux->next);		
	aux->next = (struct zombie_queue *) malloc (sizeof(struct zombie_queue));
	if (aux->next==NULL)
		kpanic("EXIT NOTIFY: no hay memoria para crear un nodo en la lista zombie_queue\n");
	aux = aux->next;

	aux->ppid = actual->ppid;
	aux->task_struct = actual;
	aux->next = NULL;
	
	zombie_queue_len++;
	
	return 0;
}


/*
pid_t sys_wait (int *status)
{
	_cli();
    status = convertir_direccion( status , actual->cr3_backup);

	struct zombie_queue *aux, *tmp;

	int zombie_found = 0;
	
	while (1) {
		for (aux=&zombie_header ; aux->next!=NULL ; aux=aux->next) {
			if ( aux->next->ppid == actual->pid)	{			// Encontre un hijo "zombie"
				zombie_found = 1;
				break;
			}
		}
		if (zombie_found==1)
			break;
		if (getvar("__wait")==1)
			kprintf("SYS_WAIT: Mi PID es: %d\tNo encontre hijo zombie\n", actual->pid);
		_sti();
		_reschedule();	// espero un rato
		_cli();
	}
	
	// Mientras que aux queda apuntando al nodo anterior, tmp lo hace al nodo en cuestión
	tmp = aux->next;
	pid_t child_pid = tmp->task_struct->pid;
	if (getvar("__wait")==1) 
		kprintf("SYS_WAIT:Mi hijo zombie es: %d\n", tmp->task_struct->pid);

	// Valor de retorno del hijo
	*status = tmp->task_struct->retorno;
	remover_task (tmp->task_struct);
	
	// Quitar al nodo de la lista
	aux->next = tmp->next;
	// Liberar el task struct utilizado
	kfree_page ((addr_t)(tmp->task_struct));
	free (tmp);
	zombie_queue_len--;
	
	_sti();
	return child_pid;
}
*/
/*! \brief  llamada al sistema waitpid
 * \param	dirección de un entero donde se colocará el valor de salida del hijo
 * \return	PID del hijo
 */

#define WNOHANG 1

pid_t sys_waitpid (pid_t pid, int *status, int options)
{
	_cli();
    status = convertir_direccion( status , actual->cr3_backup);

	struct zombie_queue *aux, *tmp;

	int zombie_found = 0;
	
	while (1) {
		// Recorrer la lista de procesos zombies
		for (aux=&zombie_header ; aux->next!=NULL ; aux=aux->next) {
			if ( aux->next->ppid == actual->pid)	{			// Encontre un hijo "zombie"
				// Buscar cualquier hijo (pid=0) o alguno en particular (pid>1)?	
				if (pid>1  &&  pid!=aux->next->task_struct->pid)
					break;
				zombie_found = 1;
				break;
			}
		}
		if (zombie_found==1)	
			break;
		if (getvar("__wait")==1)
			kprintf("SYS_WAIT: Mi PID es: %d\tNo encontre hijo zombie\n", actual->pid);
		// Si es no bloqueante, debo irme con -1 al no haberlo encontrado
		if (options==WNOHANG)	{		
			if (getvar("__wait")==1)
				kprintf("SYS_WAIT: Me voy por el WNOHANG....\n");
			actual->err_no = ECHILD;
			_sti();
			return -1;
		}
		_sti();
		_reschedule();	// espero un rato
		_cli();
	}
	
	// Mientras que aux queda apuntando al nodo anterior, tmp lo hace al nodo en cuestión
	tmp = aux->next;
	pid_t child_pid = tmp->task_struct->pid;
	if (getvar("__wait")==1) 
		kprintf("SYS_WAIT:Mi hijo zombie es: %d\n", tmp->task_struct->pid);

	// Valor de retorno del hijo
	*status = tmp->task_struct->retorno;
	remover_task (tmp->task_struct);
	
	// Quitar al nodo de la lista
	aux->next = tmp->next;
	// Liberar el task struct utilizado
	kfree_page ((addr_t)(tmp->task_struct));
	free (tmp);
	zombie_queue_len--;
	
	_sti();
	return child_pid;
}













//Cantidad de veces que la función malloc llamo a morecore solicitandole una página
extern word morecores;
extern struct floppy_cache *header_floppy_cache;
/*! \brief muestra parametros internos con propósitos de debug
 *  \param parametro que desea mostrarse
 */

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
		case 3:
			kprintf ("Cantidad de procesos zombies: %d\n", zombie_queue_len);
			struct zombie_queue *tmp;
			for (tmp=zombie_header.next ; tmp ; tmp=tmp->next) {
				kprintf("Proceso Zombie PID: %d\tNombre: %s\n", tmp->task_struct->pid,tmp->task_struct->descripcion);
			}
			break;
		default:
			break;
	}
			
}

