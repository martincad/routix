/* file.c */
#include "../include/system.h"
#include "../include/string.h"
#include "../include/fat.h"
#include "../include/paging.h"
#include "../include/kalloc.h"
#include "../include/stdio.h"
#include "../include/file.h"
#include "../include/task.h"
#include "../include/drivers/floppy.h"

file_opened_t *header_archivos_abiertos;

extern dev_fat_t dev_fat[0];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funcion auxiliar usada para buscar en la lista enlazada, el file descriptor fd
// en caso de no encontrarse devuelve NULL, en caso contrario devuelve un puntero al nodo correspondiente
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Header a una lista enlazada con todos los archivos abiertos por el sistema
file_opened_t *header_archivos_abiertos=NULL;

// Como abrir_archivo va a retornar un fd, creo una variable dword file_descriptor_actual, la cual va incrementando
// el fd por cada archivo abierto. Es decir, el primer fd que entregaria seria el 1

dword file_descriptor_actual=1;	

// La idea es que haya una lista enlazada con todos los archivos abiertos en el sistema (independientemente a que
// cada proceso tengo en su estructura de datos info de los archivos que abrio.
// Cada nodo de la lista, contiene una estructura fat12_entry_ext_t datos, un puntero a un bloque de memoria (que sera
// levantado de disco por la funcion read, y un dword sector_actual que indica a que sector corresponde el bloque
// apuntado por byte *bloque.
// Si bien es ineficiente, para hacer bien claro el manejo, se levantara de disco de a 1 sector por vez.


extern task_struct_t *actual;

int open (char *nombre)
{
    word i;
    int fd=-1;


    
    // Verificar que el proceso tenga algun slot libre para abrir un archivo
    for (i=0 ; i < MAX_FILES_POR_TAREA ; i++)
	if ( actual->open_files[i] == NULL ) {
	    fd = i;
	    break;
	}
	
    if ( fd < 0 )
	return -1;	//el proceso posee la maxima cantidad de archivos abiertos, no se puede abrir otro
    
    struct file aux;

/*  Esta funcion me devuelve en que dispositivo (aux.device) y en que file system (aux.fs) se encuentra el archivo
    vfs_find(nombre, aux);	
    como el file system virtual y el sistema de montajes no esta implementado, completo los valores a mano.
*/			
    aux.device = DRIVE_0 ;
    aux.fs = FAT12;

    actual->open_files[fd] = (struct file *) malloc (sizeof(struct file));
    if ( ! actual->open_files[fd] )
	return -1;

    
    if ( aux.fs == FAT12 ) 
	if ( open_FAT12(nombre, fd)!=OK ) {
	    free(actual->open_files[fd]);
	    actual->open_files[fd] = NULL;
	    return -1;
	}
	    
    return fd;
    
    
}	

// Abrir un archivo desde un dispositivo FAT12
int open_FAT12 (char *nombre, int fd)
{
    if ((dev_fat[0].boot_leido==FALSE) || (dev_fat[0].fat_levantada==FALSE) ) 
        if ( init_floppy_fs() != OK ) {
	    kprintf("No se puede leer disco\n");
	    return -1;
	}
    
    fat12_entry_ext_t archivo;

    
    if ( fat_file_find(nombre , &archivo.fat12_data) == NULL ) 	//Archivo no se encuentra o no se puede abrir
	return -1;
    
    actual->open_files[fd]->sectores = archivo.fat12_data.size / SECTOR_SIZE;

    if ( archivo.fat12_data.size % SECTOR_SIZE )	    //Si no es un sector justo, aumentarlo en 1
	actual->open_files[fd]->sectores++;

    actual->open_files[fd]->offset = 0;
    actual->open_files[fd]->offset_rel = 0;
    actual->open_files[fd]->sector_origen = archivo.fat12_data.sector + dev_fat[0].root_dir_start + \
		dev_fat[0].root_dir_size - 2;
    actual->open_files[fd]->sector_actual = actual->open_files[fd]->sector_origen;

    actual->open_files[fd]->size = archivo.fat12_data.size;
    
    return OK;
}	



int lseek (int fd, int offset, int donde)
{
    
    struct file *aux = actual->open_files[fd];

    dword sector_aux;	// Numero de orden del sector (no es el numero del sector en el dispositivo. Toma los valores
			// 0,1,2,3,4,5, etc dependiendo del sector sobre el cual este el offset
    if (aux == NULL)
	return -1;


    if (donde == SEEK_SET) {		//Posicionarse relativo al comienzo
      
	aux->sector_actual = aux->sector_origen;    //Apuntar al primer sector (corrigiendolo)
	if (offset < 0) {
	    aux->offset = 0;
	    return 0;		    //Posiciono al origen
	}

	sector_aux = offset / SECTOR_SIZE;

	// Recorrer la FAT en busca del sector del proxumo sector
	while ( sector_aux--  ) {
	    aux->sector_actual = fat_next_sector(aux->sector_actual);
	    if (aux->sector_actual <= 31)	//provisorio
		return 0;	//El offset esta mas alla del tamaño del archivo
	}

	aux->offset = offset;
	aux->offset_rel = offset % SECTOR_SIZE;	    //Offset relativo al comienzo del sector actual
	return offset;
	
    }
	
    else if (donde == SEEK_CUR) {

	offset = aux->offset + offset;	//Aca obtuve el offset relativo al comienzo
	return lseek(fd, offset , SEEK_SET);	    //AMO LA RECURSIVIDAD !!! no puedo creer que despues de escribir
    }						    //tanto codigo repetido e ineficiente, me haya iluminado

    else if (donde == SEEK_END) {
	if (offset > 0) 
	    return 0;
	
	offset = aux->size + offset;	// Recordar que en este caso el offset seria negativo
	return lseek(fd, offset , SEEK_SET);
    }

    else return aux->offset;
    
}	




int close (int fd)
{
    struct file *aux;
    if ( (aux = actual->open_files[fd])==NULL )
	return -1;
    
    free(actual->open_files[fd]);
    actual->open_files[fd]=NULL;
    return fd;
}


ssize_t read (int fd, void *buf, size_t nbytes)
{
    size_t len=nbytes;
    ssize_t aux_len;
    
    struct file *aux;	    //Apunta a la estructura de archivo abierto
    
    if ( (aux=actual->open_files[fd] )==NULL)	    //Archivo no esta abierto
	return -1;

    //Corregir len en caso de que sea mayor que el tamaño del archivo
    if ( (aux->size) < (len + aux->offset) )
	len = (aux->size) - aux->offset;

    aux_len = len;
    
    char *bloque;   //Puntero a bloque que contiene el sector ( mantenido por floppy_cache)

    //Esta variable me indica a cuantos bytes estoy del final del sector actual
    word remanente = SECTOR_SIZE - aux->offset_rel;
    word desplaz=0;
    

    //Este ciclo va a ser el encargado de ir leyendo el sector que corresponde, y poniendolo en el buffer (buf)
    while (len > 0) {
        bloque = floppy_cache( aux->sector_actual );
        if (bloque == NULL) //Si no se pudo levantar al cache ese bloque...
	    return -1;

	if (len <= remanente)	//Si lo que voy a leer es menor que lo que falta para el final del sector
	    aux_len = len;	//no es necesario que cargue lea desde el disco (o desde el cache) es sector siguiente

	else aux_len = remanente;   //Si en cambio, lo que tengo que leer es mayor a lo que queda de este sector
				    //voy a tener que ir cargando del disco (o del cache) otros sectores hasta completar
				    //el total

	//Poner en el buffer del llamante lo que lei del bloque actual
	memcpy( (buf + desplaz), (bloque + aux->offset_rel), aux_len);
	
	//Corregir el offset
	lseek(fd, aux_len, SEEK_CUR );
	desplaz = desplaz + aux_len;
	len = len - aux_len;
        remanente = SECTOR_SIZE - aux->offset_rel;
    }
    return desplaz;
}

