/* task.c */

#include "../../include/system.h"
#include "../../include/task.h"
#include "../../include/paging.h"
#include "../../include/file.h"

// Un solo tss
tss_t	tss;

void inicializarTss(tss_t *tss, word cs, word ds, dword eip, dword esp, dword eflags)
{
 tss->ss0 = DESC_DATA;
 tss->cs=cs;
 tss->ds=ds;
 tss->es=ds;
 tss->fs=ds;
 tss->gs=ds;
 tss->ss=ds;
 tss->eip=eip;
 tss->esp=esp;
 tss->eflags=eflags;
 tss->ldt=0;
 tss->t=0;
 tss->cr3=POSICION_DIR_PAGINAS;
}


//Agregado de ebp para facilitar el fork
dword *inicializar_task(dword *stack, word cs, word ds, dword eip, dword esp, dword eflags)
{
	
 /* Recordemos como se pushean los datos al stack
	interrupcion
	push eax
	push ebx
	push ecx
	push edx
	push ds
	push es
	push fs
	push gs
	push ebp
	push esi
	push edi

	o sea, tenemos en el siguiente orden:
	Interrupción :	ss, esp, eflags, cs, eip
	Push manuales:  eax, ebx, .... , edi

	debemos cargar el stack del kernel de la misma manera
 */


 stack--;
 *stack-- = ds;	    //ss
 *stack-- = esp;
 *stack-- = eflags;
 *stack-- = cs;
 *stack-- = eip;
 *stack-- = 0;      //ebp
 *stack-- = 0;	    //esi
 *stack-- = 0;	    //edi
 *stack-- = 0;	    //edx
 *stack-- = 0;	    //ecx
 *stack-- = 0;	    //ebx   
 *stack-- = 0;	    //eax
 *stack-- = ds;	    //ds
 *stack-- = ds;	    //es
 *stack-- = ds;	    //fs
 *stack = ds;	    //gs
 
 return stack;
}

/*
dword *inicializar_fork(dword *stack,word cs,word ds,dword eip,dword esp,dword eflags,dword ebp, dword eax, dword ebx, \
		dword ecx, dword edx, dword edi, dword esi)
{
 stack--;
 *stack-- = ds;	    //ss
 *stack-- = esp;
 *stack-- = eflags;
 *stack-- = cs;
 *stack-- = eip;
 *stack-- = eax;    //eax
 *stack-- = ebx;    //ebx   
 *stack-- = ecx;    //ecx
 *stack-- = edx;    //edx
 *stack-- = ds;	    //ds
 *stack-- = ds;	    //es
 *stack-- = ds;	    //fs
 *stack-- = ds;	    //gs
 *stack-- = ebp;    //ebp
 *stack-- = esi;    //esi
 *stack = edi;	    //edi
 
 return stack;
}
*/	


word get_new_pid(void)
{
 static word pid=1;
 
 return(pid++);
}



// Contador de tareas activas ( estado == TASK_RUNNING )
static int tareas_activas=0;

void inc_tareas_activas()
{
 ++tareas_activas;
}

void dormir_task(task_struct_t *tarea)
{
 task_struct_t *tarea_init;

 // No puede ser interrumpida por el TimerTick (Atomicidad)
 __asm__ __volatile__(" pushf; cli");

 //tarea_init = tareas_inicio->proxima;
 tarea_init = tareas_inicio;

 // Cambiamos su estado y le sacamos el resto de tiempo de cpu que le faltaba
 tarea->estado  = TASK_INTERRUMPIBLE;
 tarea->cuenta=tarea->prioridad;

 --tareas_activas;

 if ( tareas_activas < 1 && tarea_init->estado!=TASK_RUNNING) {
   tarea_init->estado=TASK_RUNNING;
   tareas_activas++;
 }

 __asm__ __volatile__(" popf");

}

void despertar_task(task_struct_t *tarea)
{

 task_struct_t *tarea_init;

 // No puede ser interrumpida por el TimerTick (Atomicidad)
 __asm__ __volatile__(" pushf; cli");

 //tarea_init = tareas_inicio->proxima;
 tarea_init = tareas_inicio;
  
 tarea->estado = TASK_RUNNING;

 tareas_activas++;

 if ( tareas_activas > 1 && tarea_init->estado==TASK_RUNNING ) {
   tarea_init->estado=TASK_STOPPED;
   tareas_activas--;   
 }

 __asm__ __volatile__(" popf");
 
}




task_struct_t *init_new_task(word cs, word ds, dword eip, dword esp, dword eflags, char *descripcion, word prioridad)
{
 task_struct_t *nueva;
 dword esp0;
 
  // Alocamos una página completa (para el descriptor de tarea y el stack en modo kernel).
 if ( (nueva = (task_struct_t *) kmalloc_page()) == NULL ) {
  return NULL;
 }

  // Selector de Datos (DPL=0) y esp0 al final de la página
 esp0 = (dword) nueva + 4092;

 // Crear un directorio de paginas y alojar su dir en cr3
 if ( (nueva->cr3 = (addr_t) make_pdt()) == NULL ) {
    kfree_page(nueva);
    return NULL;
 } 

 nueva->cr3_backup = nueva->cr3;

 nueva->pid = get_new_pid();
 strcpy( nueva->descripcion, descripcion);

 // Ponemos la tarea a correr
 //nueva->estado = TASK_RUNNING;
 // Stoppeada hasta no terminar todo el proceso de creacion, la debe despertar otra funcion
 // como por ejemplo sys_exec()
 nueva->estado = TASK_STOPPED;

 nueva->prioridad = prioridad;

 nueva->err_no = 0;

 // Cantidad de ticks que tiene permitido usar la cpu en forma continua
 nueva->cuenta	   = prioridad;
 nueva->tiempo_cpu = 0;

 // Inicializamos la estructura que contiene el contexto de hardware de la tarea
 nueva->esp0 = (dword) inicializar_task((dword *)esp0,cs,ds,eip,esp,eflags);	//el ultimo 0 es ebp (provisorio)

    word j;
    for (j=0 ; j<MAX_FILES_POR_TAREA; j++)
	nueva->open_files[j] = NULL;
 
 // Insertamos la tarea en la lista
 if ( ! insertar_tarea(nueva) ) {
 
  // No pudimos insertarla !! liberamos la memoria
  kfree_page(nueva);

  return NULL;
 }

 // Si hay más de una tarea activa ponemos a dormir al idle task
 //if ( ++tareas_activas > 1 ) {
 // dormir_task( tareas_inicio );
 //}

 return nueva;
}







int insertar_tarea(task_struct_t *nueva)
{
 task_struct_t *tmp;
	
 if ( nueva == NULL ) { return 0; }

 cli();

 // Nos paramos al ppio de la lista
 tmp = tareas_inicio;

 if ( tareas_inicio == NULL ) { tareas_inicio = nueva; }

 else {
 
   // Buscamos la última tarea
   for ( tmp = tareas_inicio; tmp->proxima != NULL ; tmp = tmp->proxima );
   	
   // Nos colgamos de ella
   tmp->proxima = nueva;
 }

 // La nueva tarea queda apuntando a NULL
 nueva->proxima = NULL;

 sti();
 return 1;
}



task_struct_t *encontrar_proceso_por_pid(pid_t pid)
{
 task_struct_t *tmp;

 for( tmp = tareas_inicio; tmp  && (tmp->pid != pid); tmp = tmp->proxima) ;

 return tmp;
}


/* Esta funcion toma un path de archivo, y coloca en nuevo solo el nombre del archivo.
 * Ej: viejo[]="/files/rotor.bin"  --->   nuevo[]="rotor.bin"
 * (usado por ejemplo en el "ps")
 */ 

void tomar_nombre_tarea (const char *viejo, char *nuevo)
{
    char *aux = (char *) (viejo + strlen(viejo));
    int i=12;	//Cantidad maxima de caracteres de un archivo (11 ya que estamos en FAT)
    for ( ; (aux > viejo) && (*aux!='/') && i>0 ; aux--);

    strcpy(nuevo, aux);
    nuevo = str_to_lower (nuevo);
}	

