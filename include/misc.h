/* misc.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif


// Setea el valor de una variable visible desde el modo Kernel y User
int setvar(char *nombre, int valor);
// Devuelve el valor de una variable visible desde el modo Kernel y User
int getvar(char *nombre);

// Wrappers que transforman las direccion de nombre del User al Kernel space
inline int sys_setvar(char *nombre, int valor);
inline int sys_getvar(char *nombre);

int find_var(char *nombre);
int find_empty_var(void);



