/* event.c */


#include "event.h"

// Puntero al inicio de la lista de eventos
event_t *event_inicio=NULL;

int insertar_evento(event_t *nuevo)
{
 event_t *tmp;
	
 if ( nuevo == NULL ) { return 0; }

 // Nos paramos al ppio de la lista
 tmp = event_inicio;

 if ( event_inicio == NULL ) { event_inicio = nuevo; }

 else {
 
   // Buscamos la última tarea
   for ( tmp = event_inicio; tmp->proximo != NULL ; tmp = tmp->proximo );
   	
   // Nos colgamos de ella
   tmp->proximo = nuevo;
 }

 // La nueva tarea queda apuntando a NULL
 nuevo->proximo = NULL;

 return 1;
}


// Función crítica, tengo que agregarle la deshabilitación de 
// interrupciones (bajo análisis)
int remover_evento(event_t *event)
{
 event_t *tmp;

 // Es el primer event ?
 if ( event == event_inicio ) {
   event_inicio = event->proximo;
   // Lo tengo que reemplazar por la constante correcta según la definición de errno.h
   return 0;
 }
 
 // Buscamos nuestro event entonces
 for ( tmp=event_inicio; (tmp->proximo != event) && (tmp != NULL) ; tmp = tmp->proximo ) ;

 // Si no encontramos el event devolvemos error
 if ( tmp == NULL ) {
   // Lo tengo que reemplazar por la constante correcta según la definición de errno.h
   return -1;
 }
 else {
   tmp->proximo = event->proximo;
 }

 // Lo tengo que reemplazar por la constante correcta según la definición de errno.h
 return 0;
}


void actualizar_eventos(device_t dispositivo)
{
 event_t *tmp;

 // Chequeamos si hay events activos
 if ( event_inicio == NULL ) {
  return;
 }

 for ( tmp=event_inicio; tmp != NULL; tmp = tmp->proximo ) {

   //kprintf("Evento: %x - Tipo: %d\n", tmp, tmp->dispositivo);
	 
   if ( tmp->dispositivo == dispositivo ) {

     //kprintf("Despertando a la bella durmiente: 0x%x\n", tmp->proceso);
     despertar_task(tmp->proceso);
   }
		 
 }
 
}
