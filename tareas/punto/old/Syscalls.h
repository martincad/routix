/* SysCalls.h */


#define MAX_SYSCALLS	10

// Definiciones de grupos
#define SYS_PROCESS	0
#define SYS_CONSOLE	(1 << 16)
#define SYS_FLOW	(2 << 16)
#define SYS_TIMER	(3 << 16)

// Funciones de grupo PROCESS
#define SYS_EXEC	0
#define SYS_VOID	1 
#define SYS_FORK	2

// Funciones de grupo CONSOLE
#define SYS_PRINT	0
#define SYS_GETS	1
#define SYS_CLRSCR	2

// Funciones de grupo TIMER
#define SYS_SLEEP	0

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

// Grupo Timer
int sys_sleep(int segundos);


