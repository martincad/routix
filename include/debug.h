/* debug.h */


#define __VMDEBUG  0			// Kernel Virtual Memory Debug
#define __KMM_DEBUG 0			// Kernel Memory Manager Debug
#define __8254_DEBUG 1			// 8254 Debug



#define _debug(valor) 		\
__asm__ __volatile__ (		\
	"sigue: \n\t"		\
	" movw %0, %%ax\n\t"	\
        " decw %%ax\n\t"	\
	" jnz sigue\n\t"	\
	 : : "i" (valor) : "ax"  );

#define __debug() __asm__ __volatile__ ("push %eax; \
										movl $1, %eax; \
										debug1: cmp $1, %eax; \
										je debug1; \
										pop %eax");
