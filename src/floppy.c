/* floppy.c */
#include "../include/floppy.h"
#include "../include/atomic.h"



int block(void);
int floppy_get_result(void);
int leer_sec(byte cara, byte cilindro, byte sector , byte *buffer);

//Contiene el estado del motor (0=apagado , 1=encendido)
byte motor_status=0;

byte floppy_calibrado=0;

//Array con los valores de estados y pcn en la fase de resultado.
// Valores validos: 0-255   ;	ERR_TIMEOUT	; NO_VALIDO
int status[12];	

// En esta direccion (640K - 1024) se ubica el bloque para transferencias DMA. La ubicacion de este bloque no puede
// quedar entre dos paginas (el tamaño de estas paginas es de 64K) es decir tiene que estar totalmente contenido dentro
// del mismo bloque.
addr_t dma_address=0xA0000 - 1024;

#define ST0 status[0]
#define ST1 status[1]
#define ST2 status[2]
#define ST3 status[3]

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


void set_dor(word valor )
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
	    status[posicion]=inportb(DATA);
	    posicion++;
	    continue;
	}
	//Si el controller dejo de estar ocupado (CMD_BUSY=0), no hay mas resultados que levantar, salida Ok
	else if ( msr == RQM )
	{
	    status[posicion]=NO_VALIDO;
//	    kprintf("\nCantidad de resultados obtenidos: %d\n", posicion);
	    return OK;
	}
    }

    // Si timeout llego a 0 no se pudo completar la escritura
    //return  timeout ? valor : -1;
 
    return ERR_TIMEOUT; 

}


int init_floppy()
{
    int i;

     // Reset
    outportb(DOR, 0);
    
    outportb(DOR, 0x0C);

    // Programamos la velocidad de datos (500 Kbps para 1.44MB)
    outportb(CCR, DR_500KBPS);

    //Esperamos la interupcion
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
/* Imprimir el resultado de la operacion
    byte posicion=0;
    while ( status[posicion] != NO_VALIDO && posicion < 12)     {
        kprintf("Resultado %d: %x\n", (posicion), status[posicion]);
	posicion++;
    }*/
 }

    
 // Ahora reconfiguramos el controlador
 if ( ! floppy_sendbyte(CONFIGURE_0) ) {
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(CONFIGURE_1) ) {
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(CONF_EIS|CONF_EFIFO|CONF_POLL) ) {
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(0) ) {
	return ERR_TIMEOUT;
 }

 // Seguimos con SPECIFY
 if ( ! floppy_sendbyte(SPECIFY) ) {
	return ERR_TIMEOUT;
 } 

 if ( ! floppy_sendbyte(0xDF) ) {			// Seteamos SRT y HUT
	return ERR_TIMEOUT;
 }
 if ( ! floppy_sendbyte(0x2) ) {		// Seteamos HLT y ND (Ahora esta en NON-DMA !!)
	return ERR_TIMEOUT;
 }

   
}


////////////////////////////////////////////////////////////////////////////////////////
// Recalibra la posicion de la cabeza a la pista 0
////////////////////////////////////////////////////////////////////////////////////////
#define PCN	status[1]

int recalibrate()
{
	 
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
   
    if ( (ST0 != (ST0_DRIVE_A | ST0_IC_OK | ST0_SEEK)) || (PCN!=0) )	{
	return ERR_NEED_RESET;
    }
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
    
    //Por comportamiento dispar de controladores (o mala interpretacion mia) voy a enmascarar el bit de Head
    //ya que a veces lo setea al valor que corresponde y a veces no
    if ( (ST0 & 0xF8) != (ST0_DRIVE_A |  ST0_SEEK ))	{
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


    // Verifico los registros de Estado ST0-ST2
    // falta verificar ST1 y ST2 (ya que son una especificacion del error, y me complicaba el tratamiento de los ERR)
    if ( ST0 != (ST0_DRIVE_A | (cara << 2) ))	{
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

byte continuar;	    // variable candado de block

int block(void)
{
    disable_irq(6);
    continuar=0;
    dword timeout=0xffffff;	//Este timeout es momentaneo (sera reemplazado por un timer). Puede que en maquinas con
    enable_irq(6);		//micro de mas de 600Mhz haya que aumentarlo... 
    while (!continuar) {
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


    if ( seek(cara, cilindro) != OK) {
	    goto intentar_nuevamente;
    }
  
/*    
Sector = (LBA mod SectorsPerTrack)+1
Cylinder = (LBA/SectorsPerTrack)/NumHeads
Head = (LBA/SectorsPerTrack) mod NumHeads
*/

    if (operacion == READ_SECTOR) {
	if ( leer_sec(cara, cilindro, sector, buffer)!=OK )
	    goto intentar_nuevamente;
	return OK;
    }

    //No implementado
    if (operacion == WRITE_SECTOR)
	return OK;
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
