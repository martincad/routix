/* SysCalls.h */

#ifndef __SYSTEM
#include "system.h"
#endif

#ifndef __TASK
#include "task.h"
#endif


#define MAX_SYSCALLS	10

// Definiciones de grupos
#define SYS_PROCESS	0
#define SYS_CONSOLE	(1 << 16)
#define SYS_FLOW	(2 << 16)
#define SYS_TIMER	(3 << 16)
#define SYS_MEM		(4 << 16)

// Funciones de grupo PROCESS
#define SYS_EXEC	0
#define SYS_VOID	1
#define SYS_FORK	2
#define SYS_PERROR	3
#define SYS_RENICE	4
#define SYS_GET_PID	5
#define SYS_GET_PPID	6
#define SYS_EXIT	7
#define SYS_SHOW	8


// Funciones de grupo CONSOLE
#define SYS_PRINT	0
#define SYS_GETS	1
#define SYS_CLRSCR	2

// Funciones de grupo TIMER
#define SYS_SLEEP	0
#define SYS_PROC_DUMP	1
#define SYS_KILL	2
#define SYS_USLEEP	3
#define SYS_PROC_DUMP_V	4

// Funciones de grupo MEM
#define SYS_MALLOC_PAGE	0
#define SYS_FREE_PAGE	1
#define SYS_FREE_MEM	2



void syscall (void);
int sys_no_existe (dword numero);

// Funciones de libreria
int sys_putc (char);


// Grupo Console
int sys_gets (char *str);
int sys_print (void *buff, size_t largo);
int sys_clrscr (void);

// Grupo process
int sys_fork (void);
int sys_void (void);
int sys_exec (char *nombre);
void sys_perror (char *str);
int sys_renice (word pid, word prioridad);
pid_t sys_get_pid (void);
pid_t sys_get_ppid (void);
void sys_exit (int);
void sys_show (int);

// Grupo timer
int sys_sleep(int segundos);
int sys_usleep(int usegundos);
int sys_proc_dump(void);
extern int sys_kill(pid_t pid, int sig);
int sys_proc_dump_v(int);

// Grupo mem
void *sys_malloc_page(void);
int sys_free_page(void *);
dword sys_free_mem (void);

