/* teclado.c */

#include "../../include/system.h"
#include "../../include/event.h"

extern char bufferTeclado[];
extern char *ptrbufferTeclado;

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
