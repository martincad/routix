/* kalloc.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

dword kmem_free (void);
addr_t get_free_page(void);
addr_t kmalloc_page(void);

struct user_page *umalloc_page ( word flags, addr_t vdir);
struct user_page *ufree_page (struct user_page *aux);


