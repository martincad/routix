/* tarea.c */
#include "../include/routix.h"

char pepito[4000];
char str[500];

char error_types[][55] = {
    "OK",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",				
    "No such device or address",
    "Arg list too long",
    "Exec format error",
    "Bad file number",
    "No child processes",
    "Try again",
    "Out of memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Device or resource busy",
    "File exists",
    "Cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "File table overflow",
    "Too many open files",
    "Not a typewriter",
    "Text file busy",
    "File too large",			
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Math argument out of domain of func",
    "Math result not representable",
    "Resource deadlock would occur",
    "File name too long",
    "No record locks available",
    "Function not implemented",		
    "Directory not empty",
    "Too many symbolic links encountered",
    "Operation would block",
    "No message of desired type",
    "Identifier removed",
    "Channel number out of range",
    "Level 2 not synchronized",
    "Level 3 halted",
    "Level 3 reset",
    "Link number out of range",
    "Protocol driver not attached",
    "No CSI structure available",
    "Level 2 halted",
    "Invalid exchange",
    "Invalid request descriptor",
    "Exchange full",
    "No anode",
    "Invalid request code",
    "Invalid slot",
    "File locking deadlock error",
    "Bad font file format",
    "Device not a stream",
    "No data available",
    "Timer expired",
    "Out of streams resources",
    "Machine is not on the network",
    "Package not installed",
    "Object is remote",
    "Link has been severed",
    "Advertise error",
    "Srmount error",
    "Communication error on send",
    "Protocol error",
    "Multihop attempted",
    "RFS specific error",
    "Not a data message",
    "Value too large for defined data type",
    "Name not unique on network",
    "File descriptor in bad state",
    "Remote address changed",
    "Can not access a needed shared library",
    "Accessing a corrupted shared library",
    ".lib section in a.out corrupted",
    "Attempting to link in too many shared libraries",
    "Cannot exec a shared library directly",
    "Illegal byte sequence",
    "Interrupted system call should be restarted",
    "Streams pipe error",
    "Too many users",
    "Socket operation on non-socket",
    "Destination address required",
    "Message too long",
    "Protocol wrong type for socket",
    "Protocol not available",
    "Protocol not supported",
    "Socket type not supported",
    "Operation not supported on transport endpoint",
    "Protocol family not supported",
    "Address family not supported by protocol",
    "Address already in use",
    "Cannot assign requested address",
    "Network is down",
    "Network is unreachable",
    "Network dropped connection because of reset",
    "Software caused connection abort",
    "Connection reset by peer",
    "No buffer space available",
    "Transport endpoint is already connected",
    "Transport endpoint is not connected",
    "Cannot send after transport endpoint shutdown",
    "Too many references: cannot splice",
    "Connection timed out",
    "Connection refused",
    "Host is down",
    "No route to host",
    "Operation already in progress",
    "Operation now in progress",
    "Stale NFS file handle",
    "Structure needs cleaning",
    "Not a XENIX named type file",
    "No XENIX semaphores available",
    "Is a named type file",
    "Remote I/O error",
    "Quota exceeded"
};

#define nop() __asm__ __volatile__ ("nop")

void main(void) 
{
	nop();
	nop();
	nop();
	strcpy(pepito, "Estoy en un array de 4000 bytes");
	printf("%s\n", pepito);
	printf("Size del coso: %d\n", sizeof(error_types));
	perror("HOLA");
	usleep(40000000);
	printf("10: %s\n", error_types[10]);
	printf("10: %s\n", error_types[11]);
	printf("10: %s\n", error_types[12]);
	printf("10: %s\n", error_types[13]);
	printf("10: %s\n", error_types[14]);
	printf("10: %s\n", error_types[15]);
	printf("10: %s\n", error_types[16]);
	printf("10: %s\n", error_types[17]);
	printf("10: %s\n", error_types[18]);
	printf("10: %s\n", error_types[19]);
	printf("10: %s\n", error_types[20]);
	printf("Pablito clavo la madera\n\n");

	pepe();
	pepe1();
	pepe2();
	
	
	while(1);
}



void muestra_vector(int *array)
{ int i;
for (i=0;i<5;i++)
 printf("%d ",array[i]);
printf("\n");
}

void pepe()
{ int array[]={5,4,3,2,1};
  int i,j,auxiliar;
  printf("El vector inicial es: ");
  muestra_vector(array);
  for (i=0;i<5;i++)
  	for (j=i+1;j<5;j++)
	  if (array[i]>array[j]) {
	  					auxiliar=array[i];
						array[i]=array[j];
						array[j]=auxiliar;
	  						}
 printf("El vector ordenado es: ");
 muestra_vector(array);							
}
void pepe2()
{ int array[]={5,4,3,2,1};
  int i,j,auxiliar;
  printf("El vector inicial es: ");
  muestra_vector(array);
  for (i=0;i<5;i++)
  	for (j=i+1;j<5;j++)
	  if (array[i]>array[j]) {
	  					auxiliar=array[i];
						array[i]=array[j];
						array[j]=auxiliar;
	  						}
 printf("El vector ordenado es: ");
 muestra_vector(array);							
}


void pepe1()
{ int array[]={5,4,3,2,1};
  int i,j,auxiliar;
  printf("El vector inicial es: ");
  muestra_vector(array);
  for (i=0;i<5;i++)
  	for (j=i+1;j<5;j++)
	  if (array[i]>array[j]) {
	  					auxiliar=array[i];
						array[i]=array[j];
						array[j]=auxiliar;
	  						}
 printf("El vector ordenado es: ");
 muestra_vector(array);							
}


char msg1[10];
char msg2[10];
char msg3[10];
char msg[10];

char str[]="Shell v 0.0.0.1";

int j, k;

void amain(void) 
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
	    if (exec(msg1)!=0)
		perror("No pudo ejecutarse");
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

	else if (! strcmp(msg, "show 3")) {
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
