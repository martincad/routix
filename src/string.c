/* string.c */

#include "../include/system.h"
#include "../include/string.h"

int strcmp(const char *s1, const char *s2)
{
 int i;
 int retorno=-1;

 for ( i=0; (*s1 == *s2); i++, s1++, s2++) {
  if ( ( *s1 == '\0' ) | ( *s1 == '\n' ) ) {
   retorno=0;
   break;
  }
 }

 return retorno;
}


size_t strlen(const char *s)
{
 size_t cantidad = 0;
 
 for ( cantidad=0; *(s+cantidad) ; cantidad++) ;

 return cantidad;
}

char *strcpy (char *dest, const char *src) 
{
    while ( *src) {
	*dest++=*src++;
    }
    *dest='\0';
    return dest;
}

char *strncpy (char *dest, const char *src, size_t nbytes) 
{
    while ( *src && nbytes) {
	*dest++=*src++;
	nbytes--;
    }
    *dest='\0';
    return dest;
}


char *str_to_upper (char *str)
{
    char *str_ret = str;

    for ( ; *str ; str++)
	*str = (char) toupper(*str);
    return str_ret;
}	

char *str_to_lower (char *str)
{
    char *str_ret = str;

    for ( ; *str ; str++)
	*str = (char) tolower(*str);
    return str_ret;
}	


int tolower(int c)
{
    if ( (c >= 'A') && (c <= 'Z') )
	c+=32;
    return c;
}

int toupper(int c)
{
    if ( (c >= 'a') && (c <= 'z') )
	c-=32;
    return c;
}


int memcmp( const void *s1, const void *s2, dword n)
{
    byte *s22 = (byte *) s2;
    byte *s11 = (byte *) s1;
	
    while (n--) {
	if (   *s11 != *s22 )
	    return (int) (s11-s22);
	s22++;s11++;
    }
	    
    return 0;
}	

/*
void memcpy( void *dest, const void *src, dword n)
{
    byte *dest1 = (char *)dest;
    byte *src1 = (char *)src;

    while (n--)
	*dest1++ = *src1++;
        
    return dest;
}	
*/

// Decidi escribir esta funcion de este modo al ver que el tiempo de ejecucion era entre 2 y 10 veces menor que
// el clasico codigo en C, sin sacrificar ningun tipo de comprobaciones. Influye tambien que esta funcion puede ser
// usada, entre otras cosas, para copiar paginas completas como ser el caso del directorio de paginas el cual sera
// duplicado por cada nueva tarea creada.

void *memcpy( void *dest, const void *src, dword n)
{
    __asm__("push %%ecx; push %%edi ; push %%esi ;cld ; rep ; movsb ; pop %%esi ; pop %%edi ; pop %%ecx" \
		    :  : "c" ((word) n), "S" (src), "D" (dest));
    
    return dest;
}

void *memset(void *dest, int c, size_t n) 
{
	size_t i;
	char *dst = (char *) dest;
	for (i=0 ; i<n ; i++)
		dst[i] = c;
	return dest;
}
	


