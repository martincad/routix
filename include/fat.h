/* floppy.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

#ifndef __FAT12
#define __FAT12

//Macros para simplificar el uso de la funcion leer_escribir
#define leer_sector(sec,buf) leer_escribir(READ_SECTOR,sec,buf)
#define escribir_sector(sec,buf) leer_escribir(WRITE_SECTOR,sec,buf)

// Estructura utilizada para caracterizar una particion FAT
typedef struct dev_fat_t
{
    word fat_size;		//Cantidad de sectores que ocupa cada FAT
    word fat_start;		//Sector de comienzo de la FAT
    byte cantidad_de_fats;	//Cantidad de FATs por disco (geenralmente 2)
    word root_dir_start;	//Sector de comienzo del Directorio raiz
    word root_dir_size;		//Cantidad de sectores asignados al root
    dword sectores_ocultos;	
    word sectores_totales;	//cantidad total de sectores del disco
    byte sectores_por_cluster;
    word sector_size;
    byte fat_levantada;
    byte boot_leido;		//Indica si el BPB ha sido leido y los parametros han sido cargados en las variables
}dev_fat_t;


// Estructura que contiene los campos principales de un BootSector tanto en FAT12 como FAT16 (son los primeros 62 bytes)
typedef struct boot_sector_t
{
    unsigned BS_jmpBott: 24;
    qword BS_OEMName;
    unsigned BPB_BytsPerSec: 16;
    unsigned BPB_SecPerClus: 8; 
    unsigned BPB_RsvdSecCnt: 16;
    unsigned BPB_NumFATs: 8;
    unsigned BPB_RootEntCnt: 16;
    unsigned BPB_TotSec16: 16;
    unsigned BPB_Media: 8;
    unsigned BPB_FATSz16: 16;
    unsigned BPB_SecPerTrk: 16;
    unsigned BPB_NumHeads: 16;
    unsigned BPB_HiddSec: 32;
    unsigned BPB_TotSec32: 32;
    unsigned BS_DrvNum: 8;
    unsigned BS_Reserved1: 8;
    unsigned BS_BootSig: 8;
    unsigned BS_VolID: 32;
    char BS_VolLab[11];
    char BS_FilSysType[8];
}boot_sector_t;


//Estos son los datos que identifican a cada archivo dentro de un directorio
typedef struct fat12_entry_t
{
    byte nombre[8];
    byte extension[3];
    byte atributo;
    byte reservado[10];
    word hora;
    word fecha;
    word sector;	//Cluster de comienzo del archivo (relativo al final del ultimo sector de ROOT
    dword size;		//Tamaño en bytes
}fat12_entry_t;

//Esta estructura es reciente. La idea es que con *fat12_data recoja la info del directorio, y que mediante la funcion
//fat_adapta_name la escriba en formato string dentro de nombre.
#define MAX_PATH_LEN	29

typedef struct fat12_entry_ext_t
{
    fat12_entry_t fat12_data;
    char nombre[MAX_PATH_LEN];
}fat12_entry_ext_t;

struct floppy_cache
{
    dword sector;
    byte bloque[512];
    struct floppy_cache  *next;

};

//Valores que puede tomar el 1er byte del campo nombre ( estructura dir_entry_t)
//Cualquier otro valor, significa el valor del primer caracter del nombre de archivo
#define FAT_EMPTY	0	// Campo aun no usado (luego del formateo)
#define FAT_SUBDIR	0x2E	// El archivo es un directorio
#define FAT_DELETED	0xE5	// El archivo fue borrado

#define SECTOR_SIZE	512


// structura utlizada para generar una lista enlazada que apunta a bloques en memoria que contienen la FAT
// del floppy que se encuentra en la diskettera.
typedef struct fat_t
{
    byte *bloque;
    struct fat_t *next;
}fat_t;


//Tipo usado para mantener una lista enlazada con los sectores de un archivo contenidos en memoria
/*
typedef struct file_loaded_t
{
    byte *bloque;	//Apunta a un sector
    struct file_loaded_t *next;

} file_loaded_t;
*/

// Adapta un nombre de la forma fat (KERNEL  BIN) a formato string: KERNEL.BIN\0
// asi es mas facil su comparacion, impresion y usos varios
void fat_adapta_name (byte *nombre_fat, byte *adaptado);

// Recibe un nombre de archivo, y lo busca en el disco. En caso negativo retorna NULL
fat12_entry_t *fat_file_find (char *nombre, fat12_entry_t *datos_archivo);

// Esta funcion recibe como parametro un numero de sector y mediante la FAT obtiene cual es el siguiente
int fat_next_sector ( dword sector_inicial );

void *floppy_cache (dword sector);

#define DIR_FAT_VIRTUAL	    0xF8000000


#endif

