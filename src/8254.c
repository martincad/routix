/* 8254.c */

/* Rutinas de inicialización y manejo del timer 8254 */

#include "system.h"
#include "8254.h"
#include "stdio.h"
#include "debug.h"



/*! Inicializa el timer 0 del 8254. Este genera ticks
 * los cuales son usados tanto para mantener un patrón de tiempo
 * como para darle control al Scheduler */
void init_8254()
{

#if __8254_DEBUG
 puts("init_8254:\n");
 kprintf("\tCWR: 0x%x (COUNTER_0 | LSB_MSB | MODO2)\n",(COUNTER_0 | LSB_MSB | MODO2));
 kprintf("\tFrecuencia oscilador interno: %d Hz\n",FOSC);
 kprintf("\tFrecuencia de interrupcion  : %d Hz\n",FINTERRUPCION);
 kprintf("\tCuenta: 0x%x\n",CUENTA);
#endif
 // Configuramos el contador 0 en modo 2
 outportb(CWR, COUNTER_0 | LSB_MSB | MODO2);
 
 // Configuramos la cuenta para una interrupción cada 10ms
 outportb(COUNTER0, (unsigned char) CUENTA);
 outportb(COUNTER0, CUENTA>>8);

}


