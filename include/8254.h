/* 8254.h */

void init_8254();

#define FOSC		1193182		// Frec del oscilador interno
#define FINTERRUPCION	100		// Frec de interrupción (10 ms)
#define CUENTA		FOSC / FINTERRUPCION

#define BASE_8254	0x40
#define COUNTER0	BASE_8254
#define COUNTER1	BASE_8254+1
#define COUNTER2	BASE_8254+2
#define CWR		BASE_8254+3

// Control Word Register
#define COUNTER_0	0<<6
#define COUNTER_1	1<<6
#define COUNTER_2	2<<6
#define READ_BACK	3<<6
#define COUNT_LATH_CMDS 0<<4
#define LSB		1<<4
#define MSB		2<<4
#define LSB_MSB		3<<4
#define MODO0		0<<1
#define MODO1		1<<1
#define MODO2		2<<1
#define MODO3		3<<1
#define MODO4		4<<1
#define MODO5		5<<1
#define BCD		1



