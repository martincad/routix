/* 8259.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

void enable_irq (byte irqn);
void disable_irq (byte irqn);

void inicializarPics(unsigned char int1, unsigned char int2);
void mascarasPics(unsigned char mask1, unsigned char mask2);
void endOfInterrupt(void);

