/* stdlib.c */

#include "system.h"
#include "string.h"

/* Ascii to Integer
 * "1943" => 1 * 10^3 + 9 * 10^2 + 4 * 10^1 + 3 * 10^0
 */
int atoi(const char *str)
{
 int numero = 0;
 int multiplicador = 1;

 size_t largo;

 for (largo = strlen(str) - 1 , multiplicador=1; largo > -1 ; largo--, multiplicador *= 10 )
  numero += ( *(str+largo) - '0' ) * multiplicador;

 return numero;
}
