/* tarea.c */
#include "stdarg.h"
#include "../include/routix.h"

char msg[50];

char msg1[50];
char msg2[50];

char str[]="Shell v 0.0.0.2";

int j, k;
int pid;

void main(void) 
{
    puts(str);

    putchar('\n');

    while(1) {

	printf("kernel$ ");
	gets(msg);

//    printf("*%s*\n", msg);
    
	if ( ! strcmp(msg, "clear") ) {
	    clrscr();
	}
	else if ( ! strcmp(msg, "exec") ) {
	    printf("Ingrese nombre de tarea:");
	    gets(msg1);
//		pid = fork();
//		if (pid==0) {		//Shell HIJO
		    if (exec(msg1)==-1)
				perror("No pudo ejecutarse");
//			exit(-1);
//		}
//		else if (pid==-1)
//			perror("fork");
//		wait(&j);		//El shell padre recoge la condicion de salida del shell hijo
	}
	else if ( ! strcmp(msg, "echo") ) {
	    printf("Ingrese texto:");
	    gets(msg1);
	    printf("%s\n", msg1);
	}
	else if ( ! strcmp(msg, "\n") ) {
	}
	else if ( ! strcmp(msg, "ps") ) {  proc_dump(); }
	
	else if ( ! strcmp(msg, "ps count") ) {  proc_dump(); }

	else if ( ! strcmp(msg, "timers") ) {  timer_dump(); }

	else if ( ! strcmp(msg, "kill") ) {

          puts("Pid: ");
	  gets(msg1);
          puts("Senal: ");
	  gets(msg2);

	  kill(atoi(msg1), atoi(msg2));
		
	}

	else if ( ! strcmp(msg, "info") ) {
	    printf("Ingrese el PID: ");
	    gets(msg2);
	    if (proc_dump_v(atoi(msg2))==-1)
		perror("proc dump verbose"); 
	}

	else if ( ! strcmp(msg, "renice") ) {
	    printf("Ingrese el PID: ");
	    gets(msg1);
	    printf("Ingrese la nueva prioridad: ");
	    gets(msg2);
	    if (renice( atoi(msg1), atoi(msg2)) == -1)
			perror("renice");

		proc_dump();
	}
	
	else if ( ! strcmp(msg, "free mem") ) {  printf("Paginas disponibles: %d\n",free_mem() ) ; }
	
	else if (! strcmp(msg, "show morecores")) {
		show(1);
	}

	else if (! strcmp(msg, "show cache")) {
		show(2);
	}

	else if (! strcmp(msg, "show zombies")) {
		show(3);
	}

	else if (! strcmp(msg, "setvar")) {
	    printf("Ingrese el nombre: ");
	    gets(msg1);
	    printf("Ingrese el valor: ");
	    gets(msg2);
		if (setvar(msg1, atoi(msg2))==-1)
			printf("No pueden definirse más variables\n");
	}

	else if (! strcmp(msg, "getvar")) {
	    printf("Ingrese el nombre: ");
	    gets(msg1);
		j = getvar(msg1);
		if (j==-1)
			printf("%s no definida\n", msg1);
		else printf ("Valor de %s: %d\n", msg1, j);
	}

	else printf("comando o nombre de archivo erroneo\n");
    }

}    
