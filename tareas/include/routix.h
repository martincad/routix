/* routix.h */
#define NULL (void *) 0

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long qword;

typedef dword addr_t;
typedef unsigned int size_t;
typedef int ssize_t;

void gets (char *str);
void voido (void);
int putchar (char car);
void puts(char *str);
void printf ( char *string, ...);
void sprintf ( char *string, ...);
void sputchar (char car);
size_t strlen(const char *s);

int sleep(int);
int proc_dump(void);
void show(int);

size_t free_mem(void);
