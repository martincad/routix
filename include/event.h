/* event.h */


#include "task.h"

// Lista de dispositivos
typedef enum device_t { TECLADO } device_t;

typedef struct event_t {
  device_t dispositivo;
  task_struct_t *proceso;
  struct event_t *proximo;
} event_t;
