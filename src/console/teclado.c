/*!  \addtogroup Consola
	\page teclado.c		Contiene las funciones de manejo de teclado. 
*/
#include "../../include/system.h"
#include "../../include/teclado.h"
#include "../../include/video.h"
//#include "../../include/console.h"
#include "../../include/atomic.h"
#include "../../include/event.h"


/*! Puntero a tabla de caracteres (por default keymap_std) */
unsigned int *keymap = keymap_std;

/*! Buffer driver de teclado */
#define MAX_BUFF_TECLADO   16
char buff_teclado[MAX_BUFF_TECLADO + 2];
char *buff_head = buff_teclado;	    /* Apunta al lugar donde debo guardar el prox caracter */
char *buff_tail = buff_teclado;	    /* apunta al caracter que debo sacar primero */
word buff_cant = 0;

char shift=0, alt=0, ctrl=0;
char num_lock, caps_lock, scr_lock;

/*! Buffer consola, almacenado en una cola circular */
#define MAX_BUFF_CONSOLA   32
char buff_consola[MAX_BUFF_CONSOLA + 2];
char *consola_head = buff_consola;	    /* Apunta al lugar donde debo guardar el prox caracter */
char *consola_tail = buff_consola;	    /* apunta al caracter que debo sacar primero */
word consola_cant = 0;


/*! Rutina de atención de interrupción de teclado. Solo toma los scan codes del puerto correspondiente y los coloca
 * en una cola circular de MAX_BUFF_TECLADO caracteres. */
void Teclado(void) 
{
    byte scode;
    scode = inportb (TECLADO_PORT);

    if ( buff_cant >= MAX_BUFF_TECLADO ) 	/* Buffer lleno, no puede guardarse el scancode */
        goto salir_int_teclado;

    unsigned int car = _getascii (scode & 0x7F);
    
    if (scode >= 0x80 )		/* Breakcode */
	if ( car!=CTRL  &&  car!=ALT  &&  car!=SHIFT )
	    goto salir_int_teclado;	        /* Fue el release de un caracter normal... me voy */
    
    /* Se presiono alguna combinacion de ALT+F ,switchear de tty en consola*/
/*
	if ( car>=F1 && car<=F8 && alt) {
	console_switch(car-F1);
        goto salir_int_teclado;
    }
*/
    /* (TEMPORAL) Si se presiona CTRL + F, poner al shell en ForeGround */
/*	
    if (toupper(car)=='F' && ctrl) {
	shell_to_fg();
        goto salir_int_teclado;
    }
*/	    
    *buff_head = scode;
    buff_head++;
    buff_cant++;
    if (buff_head == (buff_teclado + MAX_BUFF_TECLADO) ) {
        buff_head = buff_teclado;
    }

salir_int_teclado:
    endOfInterrupt();	
}

/*! Toma los scancodes del buffer de teclado y según el estado de las teclas especiales, los transforma en 
 * caracteres ascii los cuales coloca en el buffer "buff_consola" */
void leer_buff_teclado (void)
{
    unsigned int car;
    cli();	
    while ( (consola_cant <= MAX_BUFF_CONSOLA) && (buff_cant > 0) ) {
        car = getascii (*buff_tail++);
        if (buff_tail == (buff_teclado + MAX_BUFF_TECLADO) )
	    buff_tail = buff_teclado;
	buff_cant--;

	if (car > 0xff)	    /* Si el codigo levantado del buffer pertenece a alguna tecla de función */
	    continue;	    /* o ALT, CTRL, NUM_LOCK, por ahora, no la tengo en cuenta */

	*consola_head++ = (unsigned char) car;
	consola_cant++;
        if (consola_head >= (buff_consola + MAX_BUFF_CONSOLA) ) {
	    consola_head = buff_consola;
	}
    }
//    if (consola_cant>0)
	// Actualizamos los eventos de TECLADO
  //      actualizar_eventos( TECLADO );

    sti();

}

/*! Devuleve el caracter ASCII de un determinado scan code, sin tener en cuenta teclas especiales o de funciones */
inline unsigned int _getascii (unsigned char code)
{
    return keymap[code];
}

/*! Devuelve el ASCII, teniendo en cuenta el estado de las teclas de especiales. 
 * Por ej: Si alguien presiona CTRL + m, esta función retirara primero el CTRL del buffer, y activará el flag "ctrl"
 * cuando retire el scan code correspondiente a "m", al ver que está activado "ctrl" devolverá un entero CTRL | M
 * Este tipo de combinaciones deberán ser reconocidas por la TTY */
unsigned int getascii (unsigned char code)
{
    unsigned int *_keymap = keymap;
    if (shift==1)
		_keymap = keymap_std_shift;

    char presionada = 1;
    unsigned int car = _keymap[code & 0x7F];
    if ( code >= 0x80 )	{	/* tecla fue soltada */
		presionada = 0;	
    }
    
    switch (car) {
		case ALT:
		    alt = TRUE & presionada;	    
	    	car = TECLA_MODIFICADORA;
		    break;
		case CTRL:
		    ctrl = TRUE & presionada; 
		    car = TECLA_MODIFICADORA;
	    	break;
		case SHIFT: 
		    shift = TRUE & presionada;
	    	car = TECLA_MODIFICADORA;
		    break;
		default: 
	    	car = car | alt | ctrl;
    }

    return car;    
    
}


unsigned char getchar(void)
{
    unsigned char car;
    while ( consola_cant <= 0 );
	cli();
    car = *consola_tail++;
    if (consola_tail == (buff_consola + MAX_BUFF_CONSOLA) )
		consola_tail = buff_consola;
    consola_cant--;
	sti();
	return car;
}









/*
unsigned char getchar(void)
{
 event_t *nuevo;
 char caracter;

 // Si hay un caracter en el buffer lo devolvemos
 if ( ptrbufferTeclado >= bufferTeclado ) {
   __asm__ __volatile__("cli");
   --ptrbufferTeclado;
   caracter = *ptrbufferTeclado;
   __asm__ __volatile__("sti");
   return(caracter); 
 }

 // Sino agregamos un nodo a la lista de eventos y esperamos
 
 // Alocamos un nuevo nodo en memoria
 nuevo = (event_t *) malloc( sizeof(event_t) );

 // Error al intentar alocar memoria (setear errno)
 if ( nuevo == NULL ) {
   return -1;
 }
 
 // Dispositivo TECLADO
 nuevo->dispositivo = TECLADO;
 nuevo->proceso=actual;
 
 // Finalmente lo agregamos a la lista de eventos
 insertar_evento(nuevo);

 // y nos ponemos a dormir
 dormir_task(actual);

 // Switcheamos de proceso mediante la macro _reschedule quien realiza una llamada
 // a la interrupción 0x51
 _reschedule();

 // Volvimos ! Hay por lo menos una tecla en el buffer, eliminamos el nodo de la lista de eventos
 remover_evento(nuevo);

 // Liberamos el espacio
 free(nuevo);

 // Devolvemos finalmente la tecla ingresada
 __asm__ __volatile__("cli");
 --ptrbufferTeclado;
 caracter = *ptrbufferTeclado;
 __asm__ __volatile__("sti");

 return(caracter);
}
*/
char *gets(char *s)
{
	unsigned char valor;
	char *string=s;
	char flag=1;

	while ( flag ) {
		valor = getchar();

		if ( valor == '\b' ) {
			if ( string != s ) {
				string--;
				putchar(valor);
			}
		} 
		else if ( valor == '\0' ) { flag--; }
 			
		else {
			*string++=valor;
		    putchar(valor);
			if ( valor == '\n' ) { string--; flag--; }
 		}
 
	}

	if ( string == s ) { s=NULL; }
	else { *string = '\0'; }

	return s;
}

