/* routix.h */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long qword;

typedef dword addr_t;
typedef unsigned int size_t;

void gets (char *str);
void voido (void);
int putchar (char car);
void puts(char *str);
void printf ( char *string, ...);
int sprintf(char *str, const char *string, ...);
void sputchar (char car);
size_t strlen(const char *s);

int sleep(int);
int proc_dump(void);

