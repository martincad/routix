/* timedeb.c */

/* Este archivo tiene como único fin realizar el debug de la
 * libreria time.c */

#include "../../include/time.h"
#include "../../include/timer.h"

#ifndef __SYSTEM
#include "../../include/system.h"
#endif


#define cmd(comando)	( ! strcmp(msg,comando) )

// Funciones
void dump_tm();
void bisiestos();
void local();


void time_debug(void) {

 char msg[80];
 while(1) {
   kprintf("tmdebug > ");
   gets(msg);

   if ( cmd("exit") ) { return; }
   else if ( cmd("dump") ) { dump_tm(); }
   else if ( cmd("bis")  ) { bisiestos(); }
   else if ( cmd("tod")  ) { kprintf("tod: %d\n",mktime( localtime(NULL) )); }
   else if ( cmd("localtime") ) { local(); }
   else if ( cmd("asctime") ) { asctime( localtime(NULL) ); }
   else if ( cmd("jiffies") ) { kprintf("jiffies: %d\n",jiffies); }
   else if ( cmd("excepcion") ) { dword *prueba=(dword *) 0xfffffff; *prueba=2; }

 }

}




void dump_tm()
{
 struct tm *tm;

 tm = localtime(NULL);

 kprintf(" tm_seg=%d\n tm_min=%d\n tm_hour=%d\n",tm->tm_sec,tm->tm_min,tm->tm_hour);
 kprintf(" tm_mday=%d\n tm_mon=%d\n tm_year=%d\n",tm->tm_mday,tm->tm_mon,tm->tm_year);
 kprintf(" tm_wday=%d\n tm_yday=%d",tm->tm_wday,tm->tm_yday);

}

void bisiestos()
{
 int i,counter;
 char msg[80];

 gets(msg);
 i=atoi(msg);

 for ( counter=0; counter < 10; counter++,i++) {
  if ( es_bisiesto(i) ) { kprintf("%d es bisiesto\n",i); }
  else { kprintf("%d no es bisiesto\n",i); }
 }
 
}


void local()
{
 struct tm *tm;
 time_t tod;
 
 tod = mktime( localtime(NULL) );


 tm = localtime(NULL);

 kprintf(" tm_seg=%d\n tm_min=%d\n tm_hour=%d\n",tm->tm_sec,tm->tm_min,tm->tm_hour);
 kprintf(" tm_mday=%d\n tm_mon=%d\n tm_year=%d\n",tm->tm_mday,tm->tm_mon,tm->tm_year);
 kprintf(" tm_wday=%d\n",tm->tm_wday);

 kprintf("\ntod: %d\n",tod);

 tm = localtime(&tod);

 kprintf(" tm_seg=%d\n tm_min=%d\n tm_hour=%d\n",tm->tm_sec,tm->tm_min,tm->tm_hour);
 kprintf(" tm_mday=%d\n tm_mon=%d\n tm_year=%d\n",tm->tm_mday,tm->tm_mon,tm->tm_year);
 kprintf(" tm_wday=%d\n",tm->tm_wday);
 
}
