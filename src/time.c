/* time.c */

#include "time.h"
#include "8254.h"
#include "stdio.h"

#ifndef __SYSTEM
#include "system.h"
#endif


static char dias_por_mes[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
enum { ENERO, FEBRERO, MARZO, ABRIL, MAYO, JUNIO, JULIO, AGOSTO, SEPTIEMBRE, OCTUBRE, NOVIEMBRE, DICIEMBRE };
static time_t reloj=0;


byte get_value(word registro)
{
 outportb_p(CMOS_C, registro);
 return(inportb_p(CMOS_V));
}

void set_value(word registro, byte valor)
{
 outportb_p(CMOS_C, registro);
 outportb_p(CMOS_V, valor);
}


void init_time()
{

 struct tm tm;

 int year = get_year();
 year = ( year >= 1970 ) ? 1900+year : 2000+year;
 year -= 1900;

 tm.tm_sec = get_sec();
 tm.tm_min = get_min();
 tm.tm_hour = get_hour();
 tm.tm_mday = get_dayofmonth();
 tm.tm_mon  = get_month() - 1;
 tm.tm_year = year ;
 tm.tm_wday = get_dayofweek();
 tm.tm_yday = day_of_year(tm.tm_year, tm.tm_mon, tm.tm_mday);
 tm.tm_isdst = 0;							// Cero por ahora


 // Seteamos el valor del reloj
 reloj = mktime( &tm );
 kprintf("Seteando reloj: %d\n",reloj);
}



void actualizar_reloj()
{
 reloj++;
}


struct tm *localtime(const time_t *clock)
{
 static struct tm tm;
 time_t tod;
 int tmp;

 // Valores del momento actual
 if ( ! clock ) {
  kprintf("localtime: null tod, seteando tod=reloj (%d ticks)\n",reloj);
  tod = reloj;
 }

 // Hay un tod especificado
 else {
  kprintf("localtime: tod especificado, tod=*clock (%d ticks)\n",*clock);
  tod = *clock;
 }

 // Calculamos el año
 for ( tm.tm_year=ANIO_INICIO; (tod -= SEC_PER_YEAR(1900+tm.tm_year)) > 0; tm.tm_year++ ) ;
 // Corregimos por el error en la última resta del for
 tod += SEC_PER_YEAR(1900+tm.tm_year);

 // Calculamos el mes
 for ( tm.tm_mon=MES_INICIO; (tod -= SEC_PER_MONTH(tm.tm_year,tm.tm_mon)) > 0; tm.tm_mon++ ) ;
 // Corregimos por el error en la última resta del for
 tod += SEC_PER_MONTH(tm.tm_year,tm.tm_mon);
 

 tm.tm_mday = (time_t) (tod / SEC_PER_DAY);
 tod -= (tm.tm_mday * SEC_PER_DAY);
 tm.tm_mday++;

 tm.tm_hour = (time_t) (tod / SEC_PER_HOUR);
 tod -= (tm.tm_hour * SEC_PER_HOUR);

 tm.tm_min = (time_t) (tod / SEC_PER_MIN);
 tm.tm_sec = tod - (tm.tm_min * SEC_PER_MIN);

 return &tm;
}



int day_of_year(int anio, int mes, int dia)
{
 int tmp;
 int dias=dia;

 // Recorremos los meses
 for ( tmp=0; tmp < mes; tmp++)
	dias += dias_por_mes[tmp];

 // Correcion por año bisiesto
 if ( (mes > FEBRERO) && es_bisiesto(1900+anio) ) dias++;

 return dias;
}



// Calculo del tod
// el tod representa la cantidad de segundos transcurridos desde un momento hasta el
// 1 de enero de 1970.
// Para entender la forma de calcularlo veamos un ejemplo:
// queremos calcular el tod de la fecha: 25/09/1979 07:49:15
//
// bien, primero que todo:
//
// 1979 - 1970 = 9 años transcurridos
// 70-71 -- 71-72 -- 72-73 -- 73-74 -- 74-75 -- 75-76 -- 76-77 -- 77-78 -- 78-79
// o sea que la cantidad de segundos hasta el inicio del 79 son:
//
// tod += 9 * 365 * 24 * 60 * 60
//
// bien, no olvidemos los años bisiestos, que desde el 70 al 78 son 1972 - 1976
// con lo que ajustando el tod anterior tenemos:
//
// tod += 2 * 1 * 24 * 60 * 60				dos días de ajuste
//
// Bien, ahora debemos calcular la cantidad de segundos desde el Enero de 1979 hasta Septiembre de 1979
// tod += dias_por_mes[ENERO] * 24 * 60 * 60 + dias_por_mes[FEB] * 24 * 60 * 60 + ... + dias_por_mes[AGOS] * 24 * 60 * 60
// como antes, si el año es bisiesto y SI PASAMOS POR FEBRERO debemos corregir:
// tod += 1 * 24 * 60 * 60
//
// Ahora, la cantidad de segundos desde el 1ero de Septiembre al 25 de Septiembre
// tod += 24 * 24 * 60 * 60
// 
//
// Ahora la cantidad de segundos desde las 00:00 del 25 de Septiembre a las 07:00 del mismo
// tod += 7 * 60 * 60
//
// Ahora la cantidad de segundos desde los 0 minutos hasta los 49 minutos
// tod += 49 * 60;
//
// Finalmente sumamos la cantidad de segundos:
// tod += 15
//
// y obtuvimos el tod buscado.
time_t mktime(struct tm *tm)
{
 int tmp;
 time_t tod=0;
	
 for ( tmp=ANIO_INICIO; tmp < tm->tm_year ; tmp++ )
	 tod += SEC_PER_YEAR(1900+tmp);

 for ( tmp=MES_INICIO; tmp < tm->tm_mon ; tmp++ )
	 tod += SEC_PER_MONTH(tm->tm_year,tmp);
 
 tod += (tm->tm_mday - 1) * SEC_PER_DAY + tm->tm_hour * SEC_PER_HOUR + tm->tm_min * SEC_PER_MIN + tm->tm_sec;

 return tod;
}





// Los años bisiestos son todos los divisibles por 4 suprimiendo los divisibles por 100
// y no divisibles por 400 ( la regla de calculo es válida desde 1582 ).
int es_bisiesto(int year)
{
 if ( (!(year%4)) && (year%100) ) { return 1; }
 else if ( !(year%400) ) { return 1;  }
 else return 0;
}

char *dias[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char *mes[]  = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


char *asctime(const struct tm *tm)
{

 char string[26];

 //kprintf("%s %s %d %d:%d:%d %d\n", dias[tm->tm_wday], mes[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (1900 + tm->tm_year) );
 kprintf("%s %s %d %d:%d:%d %d\n", dias[0], mes[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (1900 + tm->tm_year) );

 string[26]='\0';
}



