/* floppy.h */

#ifndef __SYSTEM
#include "../../include/system.h"
#endif


// defines generales
#define SENDBYTE_TIMEOUT 1000000
#define GETBYTE_TIMEOUT  1000000

////////////////////////////////////////////////////////////////////////////////////
// Valores de errores
////////////////////////////////////////////////////////////////////////////////////

//#define OK		    0
#define ERR_TIMEOUT	    -1
#define ERR_NEED_RESET	    -2
#define ERR_MAX_TRYS	    -3
#define NO_VALIDO	    -100

////////////////////////////////////////////////////////////////////////////////////
// Registros del controlador de floppy 82077
////////////////////////////////////////////////////////////////////////////////////
#define BASE	0x3f0
#define SRA	BASE
#define SRB	BASE+1
#define DOR	BASE+2
#define MSR	BASE+4
#define DSR	BASE+4
#define DATA	BASE+5
#define DIR	BASE+7
#define CCR	BASE+7


// Digital Output Register (DOR)
#define MOTD	1<<7 | 4
#define MOTC	1<<6 | 4
#define MOTB  	1<<5 | 4
#define MOTA  	1<<4 | 4
#define DMA   	1<<3 | 4
#define DMA_INT	DMA
#define RESET 	0
#define DRIVE_0 0
#define DRIVE_1 1
#define DRIVE_2 2
#define DRIVE_3 3

////////////////////////////////////////////////////////////////////////////////////
// Valores de los bits del Main Status Register (MSR)
////////////////////////////////////////////////////////////////////////////////////
#define RQM		1<<7
#define DIO		1<<6
#define NONDMA  	1<<5
#define CMDBUSY 	1<<4
#define DRV3BUSY 	1<<3
#define DRV2BUSY 	1<<2
#define DRV1BUSY 	1<<1
#define DRV0BUSY 	1<<0


// Configuration Control Register (CCR)
#define DR_1MBPS		3
#define DR_500KBPS		0
#define DR_300KBPS		1
#define DR_250KBPS		2

// Datarate Select Register (DSR)
#define DSR_RESET		1<<7
#define DSR_POWERDOWN		1<<6


////////////////////////////////////////////////////////////////////////////////////
// Comandos
////////////////////////////////////////////////////////////////////////////////////
#define SENSE_INTERRUPT_STATUS 	0x8

// CONFIGURE - son dos bytes que se deben enviar
#define CONFIGURE_0		0x13
#define CONFIGURE_1		0

// Opciones de CONFIGURE
#define CONF_EIS		1<<6
#define CONF_EFIFO		1<<5
#define CONF_POLL		1<<4

// SPECIFY
#define SPECIFY			0x3

// RECALIBRATE
#define RECALIBRATE		0x7

// READ SECTOR ID
#define READ_SECTOR_ID		0x4a

// READ SECTOR
#define READ_SECTOR		0Xe6 //0xe6

// WRITE SECTOR
#define WRITE_SECTOR		0x0

// SEEK
#define SEEK			0xf


////////////////////////////////////////////////////////////////////////////////////
//Definiciones relacionadas a la fase de resultados
////////////////////////////////////////////////////////////////////////////////////
#define ST0_IC_OK	    (0x0 << 6)
#define ST0_IC_ABNORMAL	    (0x1 << 6)
#define ST0_IC_NOT_EXIST    (0x2 << 6)
#define ST0_IC_ABNORMAL_POLL (0x3 << 6)
#define ST0_SEEK	    (0x1 << 5)
#define ST0_NOT_READY	    (0x1 << 3)
#define ST0_HEAD
#define ST0_DRIVE_A	    0x0
#define ST0_DRIVE_B	    0x1
#define ST0_DRIVE_C	    0x2
#define ST0_DRIVE_D	    0x3




#define BLOQUE_SIZE 512

void set_dor(word valor);


////////////////////////////////////////////////////////////////////////////////////
// Definiciones de DMA 
////////////////////////////////////////////////////////////////////////////////////

#define DMA_MASK	0xA
#define DMA_CMD		0xB
#define DMA_CH2_PAGE	0x81
#define DMA_CH2_ADDR	0x4
#define DMA_CH2_COUNT	0x5
#define DMA_READ	0x46	// Floppy --> mem
#define DMA_WRITE	0x4A	// mem --> floppy

/* Inicializa el Canal 2 del DMA (usado para transferencias del floppy */
void init_floppy_DMA (byte);


////////////////////////////////////////////////////////////////////////////////////
// Definiciones del floppy
////////////////////////////////////////////////////////////////////////////////////

#define SECTORES_POR_PISTA	18
#define CARAS_POR_PISTA		2
#define MAX_FLOPPY_TRYS		20	// Cantidad maxima de intentos de lectura/escritura antes de tirar error

int leer_escribir(byte operacion, dword sector_logico, byte *buffer);

