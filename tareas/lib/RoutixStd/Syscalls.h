/* SysCalls.h */


#define MAX_SYSCALLS	10

// Definiciones de grupos
#define SYS_PROCESS	0
#define SYS_CONSOLE	(1 << 16)
#define SYS_FLOW	(2 << 16)
#define SYS_TIMER	(3 << 16)
#define SYS_MEM		(4 << 16)
#define SYS_MISC	(5 << 16)

// Funciones de grupo PROCESS
#define SYS_EXEC		0
#define SYS_VOID		1	 
#define SYS_FORK		2
#define SYS_PERROR		3
#define SYS_RENICE		4
#define SYS_GET_PID		5
#define SYS_GET_PPID	6
#define SYS_EXIT		7
#define SYS_SHOW		8
#define SYS_WAIT		9

// Funciones de grupo CONSOLE
#define SYS_PRINT		0
#define SYS_GETS		1
#define SYS_CLRSCR		2

// Funciones de grupo TIMER
#define SYS_SLEEP		0
#define SYS_PROC_DUMP		1
#define SYS_KILL		2
#define SYS_USLEEP		3
#define SYS_PROC_DUMP_V		4
#define SYS_TIMER_DUMP		5

// Funciones de grupo MEM
#define SYS_MALLOC_PAGE	0
#define SYS_FREE_PAGE	1
#define SYS_FREE_MEM	2

// Funciones de grupo MEM
#define SYS_SETVAR		0
#define SYS_GETVAR		1	

void syscall (void);
int sys_no_existe (dword numero);

// Funciones de libreria
int sys_putc (char);

// Grupo Console
int sys_gets (char *str);
int sys_print (void *buff, size_t largo);

// Grupo process
int sys_void (void);
int sys_exec (char *nombre);
void sys_show(int);

// Grupo Timer
int sys_sleep(int segundos);
int sys_usleep(int usegundos);
int sys_proc_dump(int segundos);
int sys_kill(int pid, int sig);

// Grupo Misc
int setvar (char *nombre, int valor);
int getvar (char *nombre);
