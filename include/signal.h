/* signal.h */

#ifndef __SYSTEM
#include "system.h"
#endif

#define __SIGNAL


// N�mero de se�ales
#define SIGNUMBER	2

// Senales soportadas
#define SIGUSR1	1<<0
#define SIGUSR2	1<<1

// Estructura de cada proceso donde esta la
// informaci�n sobre las se�ales del mismo.
typedef struct signal_struct_t {
  
  dword senales_pendientes;
  dword senales_enmascaradas;
  void (*handler)(int);

} signal_struct_t;


