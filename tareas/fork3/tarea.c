/* tarea.c */
#include "../include/routix.h"

int buff[50];
char str[]="Mi nombre es martin  ";

struct LISTA
{
    char string[94];
    struct LISTA *next;
}lista;

struct LISTA *nuevo_elem (char *buff);

void main(void) 
{
    int i;
    struct LISTA *p, *header = nuevo_elem(str);
    printf("SIZE de struct: %d\n", sizeof(struct LISTA));
    p = header;
    for(i=0 ; i<600 ; i++) {
        p->next = nuevo_elem(str);
	if (! p->next) {
	    printf("No hay mas memoria disponible\n");
	    while(1);
	}
	    
	p = p->next;
    }
    
    p = header;

    while (p) {
//	printf("%s\n",p->string);
        p = p->next;
    }
    
    while(1);
}

struct LISTA *nuevo_elem (char *buff)
{
    static n=0;
    struct LISTA *aux = (struct LISTA *) malloc (sizeof(struct LISTA));
    if (! aux)
	return NULL;
    strcpy(aux->string, buff);
    char num = n + '0';
    aux->string[strlen(aux->string)] = num;
    aux->next = NULL;
    n++;
    return aux;
}


