
#define NULL (void *) 0
#define PAGINA_SIZE 4096
void *malloc(unsigned nbytes);
void free (void *ap);

typedef long Align;

typedef union header {
    struct {
	union header *ptr;
	unsigned size;
    } s;
    Align x;
} Header;

Header *morecore(void);
