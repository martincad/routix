/* file.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

#ifndef __FAT12
#include "../include/fat.h"
#endif

#ifndef __FILE
#define __FILE


typedef struct file_opened_t
{
//    byte *bloque;
    dword sector_actual;
    fat12_entry_ext_t datos;
    dword fd;		    //File descriptor
    dword offset;	    //Offset absoluto (al comienzo)
    dword offset_rel;	    //Offset relativo al sector actual
    dword sectores;	    //Cantidad de sectores que ocupa el archivo
    struct file_opened_t *next;
}file_opened_t;

ssize_t read (int fd, void *buf, size_t len);
int open (char *nombre);
int close (int fd);
int lseek (int fd, int offset, int donde);

#define SEEK_CUR    1
#define SEEK_SET    2
#define SEEK_END    3

#define DRIVE_0	    0
#define FAT12	    0

struct file 
{
    byte device;	// Tipo de dispositivo donde se encuentra el archivo
    byte fs;		// tipo de filesystem
    dword sector_origen;// Sector donde comienza el archivo en el dispositivo device con filesystem fs
    dword sector_actual;
    dword offset;	// offset desde el comienzo del archivo
    dword offset_rel;	// offset desde el sector sobre el que esta parado
    dword sectores;	// cantidad de sectores totales
    dword size;
};	

#define MAX_FILES_POR_TAREA 5

#endif
