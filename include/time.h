/* time.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

#define CMOS_C		0x70
#define CMOS_V		0x71


#define CUR_SEC		0
#define CUR_MIN		2
#define CUR_HOUR	4
#define CUR_DAYOFWEEK	6
#define CUR_DAYOFMONTH  7
#define CUR_MONTH	8
#define CUR_YEAR	9
#define STAT_REG_A	0xa
#define STAT_REG_B	0xb	

#define bcdtochar(valor)		(valor - 6 * (byte) (valor/16))
#define chartobcd(valor)		(valor % 10 + ( ( (byte) valor / 10 ) << 4 ))

#define get_hour()		bcdtochar(get_value(CUR_HOUR))
#define get_min()		bcdtochar(get_value(CUR_MIN))
#define get_sec()		bcdtochar(get_value(CUR_SEC))
#define get_dayofweek()		bcdtochar(get_value(CUR_DAYOFWEEK))
#define get_dayofmonth()	bcdtochar(get_value(CUR_DAYOFMONTH))
#define get_month()		bcdtochar(get_value(CUR_MONTH))
#define get_year()		bcdtochar(get_value(CUR_YEAR))
#define get_stat_reg_a()	get_value(STAT_REG_A)
#define get_stat_reg_b()	get_value(STAT_REG_B)

#define set_hour(valor)		set_value(CUR_HOUR, chartobcd(valor))


// Defines de funciones de tiempo
#define ANIO_INICIO	70
#define MES_INICIO	0
#define DIA_INICIO	1

#define SEC_PER_MIN		(60)
#define SEC_PER_HOUR		(60 * SEC_PER_MIN)
#define SEC_PER_DAY		(24 * SEC_PER_HOUR)
#define SEC_PER_YEAR(anio)	( ( 365 + es_bisiesto(anio) ) * SEC_PER_DAY )
#define SEC_PER_MONTH(anio,mes) ( dias_por_mes[mes] + ( ( (mes == FEBRERO) && es_bisiesto(1900+anio) ) ? 1 : 0 ) ) * SEC_PER_DAY


// Estructura tm
struct tm {
     int   tm_sec;    /* seconds after the minute - [0, 61] */
                      /* for leap seconds */
     int   tm_min;    /* minutes after the hour - [0, 59] */
     int   tm_hour;   /* hour since midnight - [0, 23] */
     int   tm_mday;   /* day of the month - [1, 31] */
     int   tm_mon;    /* months since January - [0, 11] */
     int   tm_year;   /* years since 1900 */
     int   tm_wday;   /* days since Sunday - [0, 6] */
     int   tm_yday;   /* days since January 1 - [0, 365] */
     int   tm_isdst;  /* flag for alternate daylight savings time */
};

typedef int time_t;

// Definimos los tipos
struct tm *localtime(const time_t *clock);
time_t mktime(struct tm *tm);
int day_of_year(int anio, int mes, int dia);
char *asctime(const struct tm *tm);



