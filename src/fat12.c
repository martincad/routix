/* floppy.c */
#include "../include/system.h"
#include "../include/floppy.h"
#include "../include/string.h"
#include "../include/fat.h"
#include "../include/paging.h"
#include "../include/kalloc.h"
#include "../include/stdio.h"
#include "../include/file.h"

//void printing (file_loaded_t *archivo);


// La mayoria de estas funciones estan hechas considerando un disco de 1.44MB (comportamiento impredecible en otro formatos)


//Algunas valores retornados
#define ERR_NOMEM	-1	//No hay memoria suficiente (kmalloc_page==NULL)
#define ERR_NO_FAT	-2
#define ERR_NO_BOOT	-3
#define ERR_LAST_SECTOR	-5
#define LAST_SECTOR	-100	//Ultimo sector


// Buffer donde se guardar cada bloque leido
byte buffer[512];


dev_fat_t dev_fat[1];		//Defino estructura para dos dispositivos FAT


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lee el sector logico 0 (Boot sector), y levanta todos los parametros fisicos y logicos del disco
// para que sean usados por las demas funciones
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fat_12(void)
{
    dev_fat[0].boot_leido = FALSE;
    dev_fat[0].fat_levantada = FALSE;

    //Leer Boot Sector del floppy
    if ( leer_sector(0,buffer) != OK)	{
	kprintf("No se pudo leer sector\n");
	return -1;
    }

    boot_sector_t *p= (boot_sector_t *) buffer;
    
    dev_fat[0].fat_size = p->BPB_FATSz16;
    dev_fat[0].cantidad_de_fats = p->BPB_NumFATs;
    dev_fat[0].sectores_ocultos = p->BPB_HiddSec;
    dev_fat[0].sectores_totales = p->BPB_TotSec16;
    dev_fat[0].root_dir_size = (p->BPB_RootEntCnt * sizeof(fat12_entry_t)) / SECTOR_SIZE;
    dev_fat[0].sectores_por_cluster = p->BPB_SecPerClus;
    dev_fat[0].sector_size = p->BPB_BytsPerSec;
    dev_fat[0].fat_start = 1;	    
    dev_fat[0].root_dir_start = 1 + dev_fat[0].sectores_ocultos + (dev_fat[0].cantidad_de_fats * dev_fat[0].fat_size);
    
    dev_fat[0].boot_leido=TRUE;
    return OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Levanta todos los sectores correspondientes a la FAT y los coloca en la memoria
// Debera tenerse en cuenta que al cambiar el diskette, el flag "fat_levantada" debera volverse a FALSE
////////////////////////////////////////////////////////////////////////////////////////////////////////////

fat_t fat_header;   //Header de una lista enlazada que va apuntando a bloques en memoria de 512 (fragmentos de la FAT)

int levantar_fat(void)
{
    //CUIDADO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    malloc(100);
    
    //Cantidad de paginas que requiere la FAT (en gral para 9 sectores de FAT se requieren 2 paginas)
    word fat_paginas = (dev_fat[0].fat_size * BLOQUE_SIZE) / PAGINA_SIZE;

    
    //Contador para saber cuantos sectores de la fat se levantaron del diskette
    word sec_fat_levantados=0;
    word i, j;

    if ( dev_fat[0].boot_leido == FALSE ) {
	kprintf("Debe levantarse el BOOT\n");
        return ERR_NO_BOOT;
    }

    if ( dev_fat[0].fat_levantada == TRUE ) {
        return OK;
    }

    
    //Si la division (fat_size * BLOQUE_SIZE) / PAGINA_SIZE no da entera, redondear para arriba
    if ( (dev_fat[0].fat_size * BLOQUE_SIZE) % PAGINA_SIZE)
	fat_paginas++;

    
    //Puntero a la lista enlazada de los bloques de FAT, inicializado a NULL en sus dos campos
    fat_t *aux= &fat_header;
    aux->bloque = NULL;
    aux->next = NULL;

    for(i=0 ; i < fat_paginas ; i++) {
    
	//Obtener una pagina
        if (!(aux->bloque = (byte *) kmalloc_page())) 	{
	    kprintf("No hay memoria disponible para levantar la FAT\n");
	    return ERR_NOMEM;
	}

	//Mapeo las paginas que contienen bloques en memoria virtual, para poder asi tenerlos en forma contigua
	//(nadie me asegura que los bloques entregados por kmalloc_page sean contiguos)
	kmapmem(aux->bloque, DIR_FAT_VIRTUAL + (i * PAGINA_SIZE), KERNEL_PDT, PAGE_PRES | PAGE_SUPER | PAGE_RW); 
	aux->bloque = (byte *) (DIR_FAT_VIRTUAL + (i * PAGINA_SIZE));

	//Hacer que cada nodo de la lista apunte a 512 bytes dentro de la pagina obtenida.
	for(j=0;(j<((PAGINA_SIZE/BLOQUE_SIZE))) && (sec_fat_levantados<dev_fat[0].fat_size);j++, sec_fat_levantados++ ) {
	    if (leer_sector(sec_fat_levantados + dev_fat[0].fat_start, aux->bloque)!=OK) {
		kprintf("No se pudo levantar la FAT");
		return -1;
	    }
    
	    aux->next = (fat_t *) malloc( sizeof(fat_t) );
	    aux->next->bloque = aux->bloque + BLOQUE_SIZE;
	    aux = aux->next;
	    aux->next = NULL;
    	}
    }

    //Levantar flag para que las funciones que requieran buscar algo en FAT, sepan que pueden hacerlo
    dev_fat[0].fat_levantada = TRUE;
    return OK;
}

// **************************************************************************************************************
// Prepara el entorno (carga la FAT y el BPB) para que puedan ser utilizadas las demas funciones de acceso
// a disco
// **************************************************************************************************************
int init_floppy_fs(void)
{
    dev_fat[0].fat_start = 1;
    if ( (fat_12()!=OK) || (levantar_fat()!=OK) )
	return -1;

    return OK;
}	



// **************************************************************************************************************
// Recibe un nombre en formato FAT ( KERNEL  BIN ) y lo transforma a un formato tipo string: KERNEL.BIN\0
// **************************************************************************************************************
void fat_adapta_name (byte *nombre_fat, byte *adaptado)
{
    byte x=0,z;
    strcpy(adaptado,"            ");
    for( z=0  ; z<11 ;  z++) {
	if ( (z==8) && (nombre_fat[8]!=' ') ) {
	    adaptado[x]='.';
	    x++;
	}
    	adaptado[x] = nombre_fat[z];
	if (nombre_fat[z] != ' ')
	    x++;
    }
    adaptado[x]='\0';
    
}	




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Esta funcion recibe un numero de sector (perteneciente a un archivo) y halla cual es el sector que le sigue.
//retorna el sector correspondiente, o LAST_SECTOR en caso de que el sector recibido ("sector") sea el ultimo.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fat_next_sector ( dword sector )
{
    //Si no se levanto la FAT del disco, no se puede hacer nada
    if (dev_fat[0].fat_levantada==FALSE)
	return ERR_NO_FAT;

    if (sector == 0xfff)
	return ERR_LAST_SECTOR;

    sector -= 31;	//Corrijo el sector, ya que la primera entrada disponible de la FAT apunta al sector 32, y no al 1
    
    // Tener en cuenta que FAT12 utiliza codificacion inversa. Cualquier duda recurrir a Microsoft Hardware White PAPER
    // FAT General Overview of On-Disk Format
    byte lsb, msb;

    word indice; 

    indice = (sector * 3) >> 1;
    lsb = fat_header.bloque[indice];
    msb = fat_header.bloque[indice + 1];

    word proximo_sector;

    if (sector % 2) 	    //Si es impar, tomo los tres nibbles mas altos
	proximo_sector = (((msb << 8) | lsb) & 0xfff0) >> 4;
    
    else			    //Si es par, tomo los mas bajos
        proximo_sector = ((msb << 8) | lsb) & 0x0fff;
        
    if ( proximo_sector==0xFFF )
	return LAST_SECTOR;
    
    return (proximo_sector + 31);
}    	

// **************************************************************************************************************
// Recibe un PATH que puede contener directorio y devuelve estilo mnt/files/prog.bin y devuelve el largo hasta
// la primera '/' o -1 en caso de que no sea una cadena de paths sino un solo nombre (estilo mnt).
// Esta funcion se usa multiples veces hasta llegar al archivo final.
// **************************************************************************************************************
int str_dir_len (char *nombre)
{
    int len=0;
    while( *nombre++ != '/') {
	len++;
	if ( (len > 12) || (*nombre=='\0') )
	    return -1;
    }

    return len;	    
}	


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	fat12_entry_t *fat_file_find (char *nombre, fat12_entry_t *datos_archivo);
//
// Esta funcion recibe un nombre de archivo, y devuelve el sector de comienzo.
// Esta version soporta directorios, asi que, cualquier funcion o comando que la llame puede pasarle como parametro
// un archivo dentro de un directorio. (mnt/files/pepe.txt)
//
// NOTA: el directorio Root tiene un tamaño fijo, y sus sectores estan contiguos. Los demas 
// directorios, tienen por tamaño 0, sus sectores pueden no ser contiguos, y para saber que en que sector continua
// el directorio debera llamarse a fat_next_sector. El flag root_dir_flag es utilizado para indicar si el directorio
// en el que se busca es el Root o algun otro (ya que el programa debera comportarse de forma diferente).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

fat12_entry_t *fat_file_find (char *nombre, fat12_entry_t *datos_archivo)
{

    char nombre_aux[MAX_PATH_LEN];    //String auxiliar donde se va parseando el nombre por tramos (entre "/")
    char nombre_archivos[MAX_PATH_LEN];    //String en el que se guarda cada nombre levantado del disco
    
    byte x; 
    byte j;

    
    byte root_dir_flag=TRUE;   //Directorio en el que se busca es el Root.
    
    //Pasar el nombre a buscar a mayusculas (recordar que la FAT solo usa mayusculas)
    str_to_upper(nombre);

    //Sector de comienzo y cantidad de sectores del directorio a leer
    dword dir_sector, dir_size;
    
    //El primer directorio a leer sera el Root Dir, asi que cargo sus parametros 
    dir_sector = dev_fat[0].root_dir_start;
    dir_size = dev_fat[0].root_dir_size;
    
    int dir_len;    //nombre actual es un archivo o directorio (ver funcion str_dir_len). Si es archivo dir_len=-1
		    //si es directorio, dir_len=cantidad de caracteres del nombre

    fat12_entry_t *aux_fat;	//Estructura para levantar los datos de cada archivo de disco
    
    do {
fat_file_find_break:
        if ((dir_len=str_dir_len(nombre)) > 0)  {	    //Si es un directorio, buscarlo y apuntar
	    strncpy(nombre_aux, nombre, dir_len);	    //nombre al proximo directorio/archivo
	    nombre = nombre + dir_len + 1;
	}
	else strcpy(nombre_aux, nombre);		    //Si llego aca, *nombre es un archivo

	do {
	   if ( leer_sector(dir_sector, buffer) != OK) {
		kprintf("No se puede leer sector\n");
		return;
	    }

	    aux_fat = (fat12_entry_t *) buffer;


	    //Recorro desde 0-16 (que son las cantidad de entradas de archivos que hay por sector)
	    for( x=0 ; x < (SECTOR_SIZE / sizeof(fat12_entry_t)) ; x++) {	
		fat_adapta_name( aux_fat->nombre , nombre_archivos);

	    	if ( strcmp( nombre_archivos, nombre_aux)==0 ) {	
		    if (dir_len < 0) {
	    		memcpy( datos_archivo, aux_fat, sizeof(fat12_entry_t));
			return aux_fat;
		    }
		    dir_sector = aux_fat->sector + 31;
		    dir_size = 0xffff;
		    root_dir_flag=FALSE;	//Ahora voy a buscar en otro directorio
		    goto fat_file_find_break;
		}
		aux_fat++;
	    }
	    if (root_dir_flag==TRUE) {
		dir_sector++;
		if (dir_sector > (dev_fat[0].root_dir_start + dev_fat[0].root_dir_size) )
		    return NULL;
	    }
	    else {
		if ((dir_sector = fat_next_sector(dir_sector))==LAST_SECTOR)
		    return NULL;
	    }

	} while (dir_sector != LAST_SECTOR);

    }while ( dir_len > 0 ); 

    //Si llegue aqui, archivo no encontrado.
    return NULL;
}












struct floppy_cache *header_floppy_cache=NULL;

dword sectores_cache=0;

// Esta funcion mantiene una lista enlazada con sectores leidos del floppy. Cualquier funcion de fs que quiera leer
// o escribir un sector, deberia recurrir a esta. (Debe controlarse perfectamente que esta funcion mantenga 
// espejados a los sectores, es decir, en algun momento debera sincronizar los sectores con el disco
/*
struct flop_cache
{
    dword sector;
    byte *bloque;
};*/
/*
#define MAX_CACHE_ENTRYS    100
struct flop_cache cache_index[MAX_CACHE_ENTRYS];
*/
void floppy_cache_init (void)
{

	return;	////////

	/*	
    word i;
    for(i=0 ; i< MAX_CACHE_ENTRYS ; i++) {
	cache_index[i].sector=0;
	cache_index[i].bloque=NULL;
    }
	*/
}

/*
void *floppy_cache (dword sector)
{
    word i;
    byte flag=FALSE;

    //kprintf("Entrando a floppy_cache\n");

    for (i=0 ; i<MAX_CACHE_ENTRYS ; i++) {
	if (cache_index[i].bloque==NULL)
	    break;
	if (cache_index[i].sector == sector) {
	    //kprintf("***Sector se encontraba en el cache***");
	    return (cache_index[i].bloque);
	}
    }	    
    
    if ( i >= MAX_CACHE_ENTRYS )	{    // Debe implementarse algun algoritmo de sustitucion
	//kprintf("***excedido de MAX_CACHE_ENTRYS en el cache***");
    	return NULL;
    }

    // Como los sectores leidos los alloco en paginas, puedo meter en cada uno de ellos PAGINA_SIZE / SECTOR_SIZE (en casos
    //  normales 8 sectores por pagina). Los indices 0,8,16,24,etc deberan pedir una nueva pagina, mientras que los sectores
    //  "internos" (1,2,3,4,5,6,7) no.
     
    if ( (i % (PAGINA_SIZE / SECTOR_SIZE))==0 )  {
        //kprintf("***Sector multiplo de: %d***", PAGINA_SIZE / SECTOR_SIZE);
        if ( (cache_index[i].bloque = (byte *) kmalloc_page()) == NULL )
	    return NULL;
	flag=TRUE;
    }
    // El sector es un sector "interno", no es necesario pedir memoria
    else {
	if ( cache_index[i-1].bloque==NULL ) {	//Hay problemas de consistencia ?
	    kprintf("Floppy cache PANIC");
	    return NULL;
	}	    
	else {	//Si todo esta bien
	    //kprintf("***Sector NO multiplo de: %d***", PAGINA_SIZE / SECTOR_SIZE);
	    cache_index[i].bloque = cache_index[i-1].bloque + SECTOR_SIZE;
	}
    }

    
    if (leer_sector( sector, cache_index[i].bloque)!=OK) {
	if (flag==TRUE)
	    kfree_page(cache_index[i].bloque);
        return NULL;
    }

    sectores_cache++;	//Fines depurativos
    cache_index[i].sector = sector;
    return cache_index[i].bloque;
    
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funcion de cache que utiliza listas enlazadas (malloc y free) la cual tiraba excepciones al cachear el sector quinceavo
// no logre averiguar porque pero intuyo que tiene que ver con la implementacion de las primitivas de malloc y free
   
void *floppy_cache (dword sector)
{
    struct floppy_cache *aux = header_floppy_cache;
    
    // Verificar si aun no se aloco ningun sector en el cache
    if ( aux==NULL ) {
        aux = (struct floppy_cache *) malloc ( sizeof(struct floppy_cache));
        if ( aux == NULL) {
	   kprintf("Malloc no me dio memoria 1\n");
	    while(1);
        }	    
	if ( leer_sector(sector, aux->bloque)!=OK ) {
	    free (aux);
	    aux->next = NULL;
	    return NULL;
	}
	aux->sector = sector;
	aux->next = NULL;
	header_floppy_cache = aux;
	sectores_cache++;
	return aux->bloque;
    }

    // Verificar si en la lista enlazada del cache se encuentra el sector buscado    	
    for (  ;  aux->next  ;  aux=aux->next ) {
        if ( aux->sector == sector ) {
	   return aux->bloque;
	}
    }

    // Este bloque es necesario para saber si el sector buscado es el ultimo nodo en la lista
    if (aux->sector == sector) {
	return aux->bloque;
    }
    
    // Si estoy aca, es que el sector no se encuentra en el cache
    aux->next = (struct floppy_cache *) malloc ( sizeof(struct floppy_cache));
    if ( aux->next == NULL) {
	kprintf("Malloc no me dio memoria 2\n");
	    while(1);
    }	    
    
    if ( leer_sector(sector, aux->next->bloque)!=OK ) {
	free (aux->next);
	aux->next = NULL;
	return NULL;
    }
    
    aux = aux->next;
    aux->sector = sector;
    aux->next = NULL;
 
    sectores_cache++;
    return aux->bloque;
    
}	


// **************************************************************************************************************
// Estas funciones solo son usadas a modo de prueba, no son parte del manejo de FAT12
// **************************************************************************************************************

void print_fat_info (void)
{
    putchar('\n');
    boot_sector_t *p= (boot_sector_t *) buffer;

    if ( p->BPB_RsvdSecCnt != 1)
	kprintf("Filesystem no reconocido\n");
    else kprintf("Disco es FAT12\n");

    byte i;
    kprintf("BS_FilSysType:");
    for(i=0; i<8;i++)
	putchar(p->BS_FilSysType[i]);
    putchar('\n');

    kprintf("Sectores por FAT: %d\n", dev_fat[0].fat_size);
    kprintf("Cantidad de FATs: %d\n", dev_fat[0].cantidad_de_fats);
    kprintf("Fat start: %d\n", dev_fat[0].fat_start);
    kprintf("Sectores ocultos: %d\n", dev_fat[0].sectores_ocultos);
    kprintf("Sectores totales: %d\n", dev_fat[0].sectores_totales);
    kprintf("Sectores por Cluster: %d\n", dev_fat[0].sectores_por_cluster);
    kprintf("Sectores asignados al directorio root: %d\n", dev_fat[0].root_dir_size);
    kprintf("Comienzo del root dir: sector %d\n", dev_fat[0].root_dir_start);
}	


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Esta funcion imprime los nombres de todos los archivos que se hallen en el Root (no tiene utilidad real)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fat_root_dir (void)
{	
    if ((dev_fat[0].boot_leido==FALSE) || (dev_fat[0].fat_levantada==FALSE) )
	if ( init_floppy_fs() != OK ) {
	    kprintf("No se puede leer disco\n");
	}
    
    byte root_dir_sector;
    byte x;
    byte j;
    fat12_entry_t *datos;

    char nombre[MAX_PATH_LEN];
    
    for (root_dir_sector=dev_fat[0].root_dir_start;root_dir_sector<(dev_fat[0].root_dir_start+dev_fat[0].root_dir_size) \
    ; root_dir_sector++) {
	if ( leer_sector(root_dir_sector, buffer) != OK) {
	    kprintf("No se puede leer sector\n");
	    return;
	}
	datos = (fat12_entry_t *) buffer;
    
	for( x=0 ; x < (SECTOR_SIZE / sizeof(fat12_entry_t)) ; x++) {	
	    if ((datos->nombre[0]!=FAT_EMPTY) && (datos->nombre[0]!=FAT_SUBDIR) && \
			    (datos->nombre[0]!=FAT_DELETED)) {
		fat_adapta_name( datos->nombre , nombre);
		kprintf("Archivo: %s\n", nombre);
	    }	
	    datos++;
	}
    }    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funcion llamada desde el shell al ejecutar "flop find"
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void fat_test(void)
{

    if ((dev_fat[0].boot_leido==FALSE) || (dev_fat[0].fat_levantada==FALSE) )
	if ( init_floppy_fs() != OK ) {
	    kprintf("No se puede leer disco\n");
	}

    byte nombre[25];
    fat12_entry_t datos_archivo;
    kprintf("Archivo a buscar:");
    gets(nombre);
    if ( fat_file_find (nombre, &datos_archivo) ) {
	kprintf("Tamaño: %d\n", datos_archivo.size);

	if (datos_archivo.atributo & 0x10)
	    kprintf("El archivo es un Directorio\n");
	
	kprintf("Comienza en Cluster: %d\n", \
	(datos_archivo.sector * dev_fat[0].sectores_por_cluster) + dev_fat[0].root_dir_start + dev_fat[0].root_dir_size -2); 
    }
    else {
	kprintf("Archivo no encontrado: %s\n", nombre);
	return;
    }
    if (dev_fat[0].fat_levantada==FALSE) {
	levantar_fat();
    }

    
    int sector=(datos_archivo.sector*dev_fat[0].sectores_por_cluster)+dev_fat[0].root_dir_start+dev_fat[0].root_dir_size-2;

    sector = fat_next_sector(sector);

    char pepe[10];
    
    while (  (sector > 0) && (sector < 0xff0) ) {
        kprintf("%d->", sector);
	sector = fat_next_sector(sector);
    }
    kprintf("FIN\n");    
}	

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Equivalente a un "cat nombre_archivo". (solo con fines de depuracion)
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void printing (file_loaded_t *archivo)
{
    word i;
    byte n=0;
    char pepe[30];
    while (archivo) {

        for (i=0 ; i<512 ; i++) {
	    if (archivo->bloque[i]=='\n')
		n++;
	    if (n==23) {
		n=0;
		kprintf("\nPresione una tecla para continuar...");
		gets(pepe);
	    }
	    putchar(archivo->bloque[i]);
	}	    
	archivo = archivo->next;
    }	    
    putchar('\n');
}	
*/
