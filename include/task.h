/* task.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

#ifndef __TASK
#define __TASK
#endif

#include "../include/file.h"

#ifndef __SIGNAL
#include "../include/signal.h"
#endif

typedef short int pid_t;

#define MAX_TASKS 7

typedef struct task_t {
	word previos_task_link;
	dword esp0;
	word ss0;
	dword esp1;
	word ss1;
	dword esp2;
	word ss2;
	dword cr3;
	dword eip;
	dword eflags;
	dword eax;
	dword ecx;
	dword edx;
	dword ebx;
	dword esp;
	dword ebp;
	dword esi;
	dword edi;
	word  es;
	word  cs;
	word  ss;
	word  ds;
	word  fs;
	word  gs;
	word  ldt;
	word  t;
	word  iomap;
} task_t;


typedef struct tss_t {
	word previos_task_link;
	word reservado0;
	dword esp0;
	word ss0;
	word reservado1;
	dword esp1;
	word ss1;
	word reservado2;
	dword esp2;
	word ss2;
	word reservado3;
	dword cr3;
	dword eip;
	dword eflags;
	dword eax;
	dword ecx;
	dword edx;
	dword ebx;
	dword esp;
	dword ebp;
	dword esi;
	dword edi;
	word  es;
	word  reservado4;
	word  cs;
	word  reservado5;
	word  ss;
	word  reservado6;
	word  ds;
	word  reservado7;
	word  fs;
	word  reservado8;
	word  gs;
	word  reservado9;
	word  ldt;
	word  reservado10;
	word  t;
	word  iomap;
} tss_t;


extern tss_t tss;
extern task_t tarea[];
void inicializarTss(tss_t *tss, word cs, word ds, dword eip, dword esp, dword eflags);

pid_t get_new_pid(void);
		
#define ltr(selector) __asm__ __volatile__("ltr %w0" : : "a" (selector));



#define MAX_PAGINAS_POR_TAREA	16
#define MAX_DESCRIPTION		64


typedef struct task_struct_t
{
    dword esp0;
    dword cr3;
    dword cr3_backup;
    pid_t pid;
    struct task_struct_t *proxima;
    char descripcion[MAX_DESCRIPTION];
    byte estado;
    word prioridad;
    word cuenta;
    dword tiempo_cpu;
    struct file *open_files [MAX_FILES_POR_TAREA];  //definido en file.h
    word num_code, num_data, num_stack;	    //Cantidad de paginas de Codigo, Datos y stack
    int err_no;
    signal_struct_t senales;
    struct user_page *mcode;
    struct user_page *mdata;
    struct user_page *mstack;
    pid_t ppid;
    int retorno;
} task_struct_t ;

enum estado { TASK_RUNNING, TASK_STOPPED, TASK_INTERRUMPIBLE, TASK_ININTERRUMPIBLE, TASK_ZOMBIE, TASK_CLEAN };



// Estructura que describe las paginas utilizadas por las tareas
#define PAGINA_DATA		0	    //pagina de datos
#define PAGINA_CODE		1	    //pagina de codigo
#define	PAGINA_STACK	0	    //pagina de stack	    
#define PAGINA_ALLOC	2	    //pagina alocada dinamicamente

struct user_page
{
    addr_t dir, vdir;
    word count;
    dword flags;
    struct user_page *next;
};




// Variables externas
extern task_struct_t *tareas_inicio;
extern task_struct_t *actual;

// Funciones para el manejo de tareas
task_struct_t *init_new_task(word cs, word ds, dword eip, dword esp, dword eflags, char *descripcion, word prioridad);
task_struct_t *encontrar_proceso_por_pid(pid_t pid);
void tomar_nombre_tarea (const char *viejo, char *nuevo);
void dormir_task(task_struct_t *tarea);
void despertar_task(task_struct_t *tarea);
int insertar_tarea(task_struct_t *nueva);
int remover_task (task_struct_t *tarea);


// Macro para reschedulear, genera una llamada a la int 0x51 quien busca la próxima tarea a correr
#define _reschedule() __asm__ __volatile__("int $0x51")


// Direcciones logicas de los segmentos del espacio de usuario
#define TASK_TEXT   0x80000000
#define TASK_DATA   0x88000000
#define TASK_STACK  0x90000000

// Ubicacion logica de una pagina de kernel que poseera permisos de usuario para contener algunos wrappers 
// (como por ejemplo, una llamada a Syscall EXIT cuando termina el main de una tarea).
// EL pedido de la pagina, y la asignacion del código de EXIT está realizado en Kmain.c
#define EXIT_TASK	(TASK_TEXT - PAGINA_SIZE)

// Estructura utilizada para almacenar una cola de procesos zombies (Siempre hay un primer nodo)
struct zombie_queue
{
	pid_t ppid;						// padre del proceso zombiee
	task_struct_t *task_struct;		// task struct del proceso
	struct zombie_queue *next;		// :-)
	
};

