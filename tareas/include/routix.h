/* routix.h */
/*
#ifndef _ROUTIX
#define _ROUTIX
*/
#define NULL (void *) 0

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long qword;

typedef dword addr_t;
typedef unsigned int size_t;
typedef int ssize_t;
typedef int pid_t;

int clrscr(void);
void gets (char *str);
void voido (void);
int putchar (char car);
void puts(char *str);
void printf ( char *string, ...);
int sprintf(char *str, const char *string, ...);
void sputchar (char car);

int sleep(int);
int proc_dump(void);
void show(int);
int renice(word pid, word prioridad);
int kill( pid_t pid, int sig);

size_t free_mem(void);
int sleep(int);
int proc_dump(void);
int proc_dump_v(int);


int setvar(char *nombre, int valor);
int getvar(char *nombre);
void perror (char *str);
int exec (char *);
void exit(int);
pid_t fork();
#define WNOHANG 1
pid_t waitpid (pid_t pid, int *valor, int options);
pid_t wait (int *valor);


//#endif
