/* floppy.c */

/*
 * Revision: 22 Marzo 2004 - Mariano
 *    + Se migra la funcion (Floppy) llamada desde la rutina de atencion a interrupcion (intFloppy) a este archivo, 
 * 	con lo que se hace la variable "continuar" (de ahora en mas llamada "floppy_continuar") como local al
 * 	archivo (static).
 *
 *    +	Se ajustan los valores de inicializacion del floppy (comandos CONFIGURE y SPECIFY) de la siguiente forma:
 * 		Se habilita la fifo (propia del 82077a) con un tama#o de 10 bytes.
 * 		Step rate interval:  4mseg
 * 		Head Unload time  : 16mseg
 * 		Head Load time    : 16mseg
 * 		
 *    + Proxima revision:
 *    		funciones de lectura y escritura
 *    		conversion de sectores logicos a fisicos (LBA)
 * ----------------------------------------------------------------
 *
 * Revision: 24 Marzo 2004 - Mariano
 *
 *    + Se crea la funcion dump_status_register() con fin de poder dumpear y debugear el estado de los registros
 *      de estado (ST0 a ST3)
 *
 *    + En la funcion read_sector se reemplaza la verificacion de todos los bits del ST0 por una macro "COMMAND_OK"
 *      (definida en include/drivers/floppy.h) que solo chequea el valor del IC de este registro de estado.
 *      Esto es ya que cuando estamos en la condicion de sector=18 el valor de "H" devuelto por ST0 cambia (de 0 a 1
 *      por ejemplo), pienso que se debera que nos indica la proxima posicion de la cabeza para leer el proximo
 *      sector (el sector numero 18 es el ultimo sector de un track en una cara).
 *      Con este cambio se observa que pueden ejecutarse sin problemas todos los archivos de disco.
 */



#include "drivers/floppy.h"
#include "atomic.h"
#include "8259.h"
#include "stdio.h"



int block(void);
int floppy_get_result(void);
int leer_sec(byte cara, byte cilindro, byte sector , byte *buffer);
void dump_states(void);
void dump_status_register(int numero, byte valor);


//Contiene el estado del motor (0=apagado , 1=encendido)
static byte motor_status=0;

static byte floppy_calibrado=0;

// Array con los valores de estados y pcn en la fase de resultado.
// Valores validos: 0-255   ;	ERR_TIMEOUT	; NO_VALIDO
static int status[12];	

#define ST0 status[0]
#define ST1 status[1]
#define ST2 status[2]
#define ST3 status[3]

// En esta direccion (640K - 1024) se ubica el bloque para transferencias DMA. La ubicacion de este bloque no puede
// quedar entre dos paginas (el tamaño de estas paginas es de 64K) es decir tiene que estar totalmente contenido dentro
// del mismo bloque.
addr_t dma_address=0xA0000 - 1024;

#define OK  0

void motor_on()
{
    outportb(DOR, DMA_INT | MOTA);
    motor_status=1;
    dword demora=0x3ffff;   
    while (demora--);		//Esperar por el regimen permanente del motor
}

void motor_off()
{
    outportb(DOR, DMA_INT);
    motor_status=0;
    floppy_calibrado=0;
}


void set_dor(word valor)
{
 outportb(DOR, valor);
}



//////////////////////////////////////////////////////////////////////////////
// Envia un byte al registro de datos del controlador                       //
//////////////////////////////////////////////////////////////////////////////
int floppy_sendbyte(byte valor)
{
 unsigned char msr;
 int timeout;
 
 for (timeout=SENDBYTE_TIMEOUT; timeout; timeout--) {
 
  // Leemos primero el MSR (Main Status Register)
  msr = inportb(MSR);

  // Si RQM=1 y DIO=0 ( Host puede escribir info)
  if ( (msr & RQM) && ~(msr & DIO)  ) {
    outportb(DATA, valor);
    break;
  }

 }

 // Si timeout llego a 0 no se pudo completar la escritura
 return timeout;
}


///////////////////////////////////////////////////////
// Lee un byte del registro de datos del controlador //
///////////////////////////////////////////////////////
int floppy_get_result()
{
 unsigned char msr;
 byte posicion=0;
 int timeout;
 
    for (timeout=GETBYTE_TIMEOUT; timeout; timeout--) 
    {
        // Leemos primero el MSR (Main Status Register)
	msr = inportb(MSR) & (RQM | CMDBUSY | DIO);

        // Si RQM=1, DIO=1 y CMD_BUSY=1 ( Host puede leer info)
	if ( msr == (RQM | CMDBUSY | DIO) )
	{
	    status[posicion++]=inportb(DATA);
	    continue;
	}
	//Si el controller dejo de estar ocupado (CMD_BUSY=0), no hay mas resultados que levantar, salida Ok
	else if ( msr == RQM )
	{
	    status[posicion]=NO_VALIDO;
	    return OK;
	}
    }

    // Si timeout llego a 0 no se pudo completar la escritura
    return ERR_TIMEOUT; 

}



// Inicializacion del floppy
int init_floppy()
{
    int i;

     // Reset
    outportb(DOR, 0);
    
    outportb(DOR, 0x0C);

    // Programamos la velocidad de datos (500 Kbps para 1.44MB)
    outportb(CCR, DR_500KBPS);

    //Esperamos la interrupcion
    block();

    // Recolectamos el estado de las interrupciones de los 4 drives
    // que soporta el controlador
    for (i=0; i<4; i++) {

	if ( ! floppy_sendbyte(SENSE_INTERRUPT_STATUS) ) {
	  return ERR_TIMEOUT;
        }
    
   
        if (floppy_get_result()==ERR_TIMEOUT)     {
	  return ERR_TIMEOUT;
        }

    }

    
 // Ahora reconfiguramos el controlador
 if ( ! floppy_sendbyte(CONFIGURE_0) ) {
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(CONFIGURE_1) ) {
	return ERR_TIMEOUT;
 }

 // COMENTARIO: me quedan dudas respecto a estos valores ya que
 // 	CONF_EIS: habilitamos los seeks automaticos (implicitos)
 // 	CONF_EFIFO: deshabilitamos los fifos !! (modo 8272a)
 // 	CONF_POLL:  deshabilitamos el polling, esto esta oks
 //if ( ! floppy_sendbyte(CONF_EIS|CONF_EFIFO|CONF_POLL) ) {}
 if ( ! floppy_sendbyte( (CONF_EIS|CONF_POLL) | 0xa) ) {		// 10 bytes de threshold
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(0) ) {
	return ERR_TIMEOUT;
 }

 // Seguimos con SPECIFY
 if ( ! floppy_sendbyte(SPECIFY) ) {
	return ERR_TIMEOUT;
 } 

 if ( ! floppy_sendbyte(0xC1) ) {			// Seteamos SRT=4mseg (0xC0) y HUT=16mseg (0x01)
	return ERR_TIMEOUT;
 }

 if ( ! floppy_sendbyte(0x8<<1) ) {		// Seteamos HLT=16mseg (0x08 << 1) y ND (Ahora esta en NON-DMA !!)
	return ERR_TIMEOUT;
 }

	return OK;   
}


////////////////////////////////////////////////////////////////////////////////////////
// Recalibra la posicion de la cabeza a la pista 0
////////////////////////////////////////////////////////////////////////////////////////
#define PCN	status[1]

int recalibrate()
{

    // Reseteamos el flag
    floppy_calibrado=0;
	 
    // Habilita el motor A y el Drive 0
    motor_on();

    // Ejecutamos un RECALIBRATE 
    if ( ! floppy_sendbyte(RECALIBRATE) ) {	
		return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(DRIVE_0) ) {			// DS0=0 DS1=0 (Drive 0)
		return ERR_TIMEOUT;
    }

    if ( block()!=OK )	{
		return ERR_NEED_RESET;
    }
    
    // Sensamos el estado de interrupcion
    if ( ! floppy_sendbyte(SENSE_INTERRUPT_STATUS) ) {
		return ERR_TIMEOUT;
    }

    // Obtener resultados de ejecucion 
    if (floppy_get_result()==ERR_TIMEOUT)     {
		return ERR_TIMEOUT;
    }
   
    // Analizamos el resultado del comando 
    if ( ! COMMAND_OK ) {
		dump_states();
		return ERR_NEED_RESET;
    }

    // Pudimos calibrarlo sin problemas
    floppy_calibrado=1;
    
    return OK;
}


byte read_sector_id (void)
{
    // Habilita el motor A y el Drive 0
    motor_on();

    // Ejecutamos un Read sector id
    if ( ! floppy_sendbyte(READ_SECTOR_ID) ) {	
	return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(0) ) {			// HD=0 (head number) , DS0=0 DS1=0 (Drive 0)
	return ERR_TIMEOUT;
    }

    block();
    
    // Sensamos el estado de interrupcion
/*    if ( ! floppy_sendbyte(SENSE_INTERRUPT_STATUS) ) {
        puts("Error, timeout enviando SENSE_INTERRUPT_STATUS\n");
	return;
    }
*/
    // Obtener resultados de ejecucion 

    if (floppy_get_result()==ERR_TIMEOUT)
    {
	return ERR_TIMEOUT;
    }

    byte posicion=0;
    while ( status[posicion] != NO_VALIDO && posicion < 12)
    {
        kprintf("Resultado Read Id%d: %x\n", (posicion), status[posicion]);
	posicion++;
    }

}	


//////////////////////////////////////////////////////////////////////////////////////
// Posicionar el cabezal sobre la pista indicada
//////////////////////////////////////////////////////////////////////////////////////

byte seek (byte cara, byte pista)
{

    // Ejecutamos un Read sector
    if ( ! floppy_sendbyte(SEEK) ) {	
        return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte( (cara << 2) |DRIVE_0) ) {			// HD=0 (head number) , DS0=0 DS1=0 (Drive 0)
        return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(pista) ) {			// Cilindro
  	    return ERR_TIMEOUT;
    }

    block();
    
    // Sensamos el estado de interrupcion
    if ( ! floppy_sendbyte(SENSE_INTERRUPT_STATUS) ) {
        return ERR_TIMEOUT;
    }

    // Obtener resultados de ejecucion 
    if (floppy_get_result()==ERR_TIMEOUT)     {
        return ERR_TIMEOUT;
    }
    
    // Analizamos el resultado del comando 
    if ( ! COMMAND_OK ) {
	dump_states();
	return ERR_TIMEOUT;
    }

    return OK;
}	

//////////////////////////////////////////////////////////////////////////////////////
// Lee un sector fisico (previamente el cabezal debe estar calibrado y posicionado)
//////////////////////////////////////////////////////////////////////////////////////

int read_sector (byte cara, byte cilindro, byte sector)
{

    init_floppy_DMA (DMA_READ);
	
    // Ejecutamos un Read sector
    if ( ! floppy_sendbyte(READ_SECTOR) ) {	
        return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(DRIVE_0 | (cara << 2)) ) {			// HD=0 (head number) , DS0=0 DS1=0 (Drive 0)
	return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(cilindro) ) {			// Cilindro
	return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(cara) ) {			// Cabeza
	return ERR_TIMEOUT;
    }

    if ( ! floppy_sendbyte(sector) ) {			// Sector
	return ERR_TIMEOUT;
    }
    if ( ! floppy_sendbyte(2) ) {			// Tamaño sector: 128 * 2^x (si x=2-->Sector=512 bytes)
	return ERR_TIMEOUT;
    }
    if ( ! floppy_sendbyte( SECTORES_POR_PISTA ) ) {			// Sectores por pista
	return ERR_TIMEOUT;
    }
    if ( ! floppy_sendbyte(27) ) {			// GAP: 27 para 3 1/2  ;    32 para 5 1/4
	return ERR_TIMEOUT;
    }
    if ( ! floppy_sendbyte(0xff) ) {			// Data Lengh
	return ERR_TIMEOUT;
    }

    if ( block()!=OK )	{
	kprintf("Error read sector block\n");
        return ERR_NEED_RESET;
    }
    
    // Obtener resultados de ejecucion 
    if (floppy_get_result()==ERR_TIMEOUT)     {
	return ERR_TIMEOUT;
    }

    // Analizamos el resultado del comando 
    if ( ! COMMAND_OK )	{
	dump_states();
	return ERR_NEED_RESET;
    }

    return OK;
}	


char read_msr()
{
 unsigned char msr;

 msr = inportb(MSR);

 return msr;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// block: bloquea la ejecucion del proceso que la llama hasta recibir una interrupcion de floppy
////////////////////////////////////////////////////////////////////////////////////////////////

static byte floppy_continuar;	    // variable candado de block

int block(void)
{
    // Deshabilitamos las interrupciones	
    disable_irq(6);

    floppy_continuar=0;

    // TEMPORAL
    dword timeout=0xffffffff;	//Este timeout es momentaneo (sera reemplazado por un timer). Puede que en maquinas con
    				//micro de mas de 600Mhz haya que aumentarlo... 
    // Las habilitamos nuevamente
    enable_irq(6); 

    // Esperamos que aparezca la interrupcion
    while (!floppy_continuar) {
	timeout--;
	if (!timeout)	//En caso de no haber llegado la IRQ6, salir con ERR_TIMEOUT
	    return ERR_TIMEOUT;
    }
    
    return OK;
}



////////////////////////////////////////////////////////////////////////////////////////////////
// Inicializa el controlador DMA, tanto para leer como para escribir (segun comando)
////////////////////////////////////////////////////////////////////////////////////////////////

void init_floppy_DMA ( byte comando )
{
    outportb( DMA_MASK, 0x06 );
    // Resetear el FlipFlop del DMA para que reconozca al primer byte enviado como un LSB
    outportb ( 0xC, 0);

    // Enviar comando (DMA_READ, DMA_WRITE)
    outportb( DMA_CMD, comando );

    // Seteo de la direccion. No debera estar por encima de los 16MB
    outportb ( DMA_CH2_ADDR, (byte) dma_address);
    outportb ( DMA_CH2_ADDR, (byte) (dma_address >> 8) );
    outportb ( DMA_CH2_PAGE, (byte) (dma_address >> 16) );

    outportb ( DMA_CH2_COUNT, (byte) ( (BLOQUE_SIZE-1) & 0xff) );
    outportb ( DMA_CH2_COUNT, (byte) ( (BLOQUE_SIZE-1) >> 8) );
    outportb ( DMA_MASK, 2);
}	




////////////////////////////////////////////////////////////////////////////////////////////////////////
// Acepta como operacion READ_SECTOR y WRITE_SECTOR (quiza pueda aceptar format).
// Es la encargada de convertir un sector logico (o LBA) en el sector fisico (CHS) correspondiente.
// Inicializa el controlador de floppy y enciende el motor. En caso de time outs, intenta hasta MAX_TRYS
////////////////////////////////////////////////////////////////////////////////////////////////////////

int leer_escribir(byte operacion, dword sector_logico, byte *buffer)
{
    byte intentos=0;
    byte cara, cilindro, sector;

    
intentar_nuevamente:	
    intentos++;
    if (intentos > MAX_FLOPPY_TRYS ) {
	kprintf("No se puede leer el sector %d(superados %d intentos)\n", sector_logico, MAX_FLOPPY_TRYS);
	return ERR_MAX_TRYS;
    }
	

    if (motor_status==0) {
	init_floppy();
        motor_on();
    }

    
    if (floppy_calibrado==0)
	if ( recalibrate()!=OK )
	    goto intentar_nuevamente;

    
    //Calcular el sector fisico (CHS) a traves del sector logico (LBA)
    cara = (sector_logico / SECTORES_POR_PISTA) % CARAS_POR_PISTA;
    cilindro = (sector_logico / SECTORES_POR_PISTA) /CARAS_POR_PISTA;
    sector = (sector_logico % SECTORES_POR_PISTA) + 1;

#if DEBUG==1
    kprintf("leer_escribir: sector logico=%d ==> h=%d c=%d s=%d\n", sector_logico, cara, cilindro, sector);
#endif
 
    // Esto no deberia ser necesario ya que configuramos el controlador con EIS (Implied Seek) 
    if ( seek(cara, cilindro) != OK) {
	    goto intentar_nuevamente;
    }
  
    if (operacion == READ_SECTOR) {
	if ( leer_sec(cara, cilindro, sector, buffer)!=OK )
	    goto intentar_nuevamente;
	return OK;
    }

    //No implementado
    if (operacion == WRITE_SECTOR)
		return OK;
	else return -1;
}	



int leer_sec(byte cara, byte cilindro, byte sector , byte *buffer)
{
    if ( read_sector(cara, cilindro , sector)!=OK )
	return ERR_NEED_RESET;
    
    word i;

    byte *dma_block = (byte *) dma_address;
    for( i=0 ; i < BLOQUE_SIZE ; i++) 
	*buffer++ = *dma_block++;

    return OK;
}

void dump_states()
{
  int posicion;

  for (posicion=0; status[posicion]!=NO_VALIDO , posicion<sizeof(status); posicion++ )
    dump_status_register(posicion, status[posicion]);


}


static char *ic_codes[] = { "Terminacion Normal", "Terminacion erronea",
		     	    "Comando invalido"  , "Terminacion erronea causada por polling" };

void dump_status_register(int numero, byte valor)
{
  switch(numero) {

    // ST0 : 7-6:IC, 5:SE, 4:EC, 3:nada, 2:H, 1-0:DS
    case 0:
	    kprintf("ST0: 0x%x - ",valor);
	    kprintf(" IC = 0x%x (%s)\n", (valor&0xc0) >> 6, ic_codes[ (valor&0xc0) >> 6 ]);
	    kprintf(" SE = 0x%x (Seek end %s)\n", (valor&0x20)>>5, ( ( (valor&0x20) >> 5) == 0 ? "erroneo" : "ok") );
	    kprintf(" EC = 0x%x (Equipment Check)\n", (valor&0x10)>>4);
	    kprintf("  H = 0x%x (Head Address)\n", (valor&0x4)>>2);
	    kprintf(" DS = 0x%x (Drive Select)\n", valor&0x3);
	    break;
 
    case 1:
	    puts("ST1:");
	    kprintf(" EN = 0x%x (End of cylinder)\n", (valor&0x80)>>7);
	    kprintf(" DE = 0x%x (Data error - CRC)\n", (valor&0x20)>>5);
	    kprintf(" OR = 0x%x (Overrun / Underrun)\n", (valor&0x10)>>4);
	    kprintf(" ND = 0x%x (No Data)\n", (valor&0x4)>>2);
	    kprintf(" NW = 0x%x (Not Writable)\n", (valor&0x2)>>1);
	    kprintf(" MA = 0x%x (Missing Address Mark)\n", valor&0x1);
	    break;

    case 2:
	    puts("ST2:");
	    kprintf(" CM = 0x%x (Control Mark)\n", (valor&0x40)>>6);
	    kprintf(" DD = 0x%x (Data Error in data field)\n", (valor&0x20)>>5);
	    kprintf(" WC = 0x%x (Wrong Cylinder)\n", (valor&0x10)>>4);
	    kprintf(" BC = 0x%x (Bad cylinder)\n", (valor&0x2)>>1);
	    kprintf(" MD = 0x%x (Missing data address mark)\n", valor&0x1);
	    break;

    case 3:
	    puts("ST3:");
	    kprintf(" WP = 0x%x (Write Protect)\n", (valor&0x40)>>6);
	    kprintf(" T0 = 0x%x (Track 0)\n", (valor&0x10)>>4);
	    kprintf(" HD = 0x%x (Head Address)\n", (valor&0x4)>>2);
	    kprintf(" DS = 0x%x (Drive Select)\n", valor&0x3);
	    break;
	    


  }


}



// Rutinas de atencion de interrupcion

// Bloqueo
void Floppy (void)
{
    floppy_continuar=1;
    endOfInterrupt();
}	
