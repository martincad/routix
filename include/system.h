/* system.h */


#define __SYSTEM

/* Constantes de memoria física y virtual */
#define KERNEL_FCODE	0
#define KERNEL_FDATA	0x100000
#define KERNEL_FSTACK_TOP 0x200000	
#define KERNEL_STACK_SIZE 0x20000
#define KERNEL_FSTACK	KERNEL_FSTACK_TOP - KERNEL_STACK_SIZE

#define KERNEL_END	KERNEL_FSTACK_TOP

#define KERNEL_CODE		0xc0000000
#define KERNEL_DATA		0xc8000000
#define KERNEL_STACK		0xd0000000
#define KERNEL_STACK_TOP	KERNEL_STACK + 0x8000000
#define KERNEL_INTERRUPT	KERNEL_STACK_TOP


#define DESC_CODE		1<<3
#define DESC_DATA		2<<3
#define DESC_CODE_USUARIO	3<<3 | 3
#define DESC_DATA_USUARIO	4<<3 | 3
#define DESC_TSS0		5<<3
#define DESC_TSS1		6<<3


#define NULL	0

#define TRUE	1
#define FALSE	0

#define OK	0

/* Definición de tipos */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long qword;

typedef dword addr_t;
typedef long size_t;
typedef unsigned long ssize_t;

 

/* Variables Globales */
extern dword memoria;

extern addr_t KERNEL_PDT;


//Kernel PANIC (suceso inesperado o irrecuperable)
void kpanic(char *str);
