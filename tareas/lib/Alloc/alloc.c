/* alloc.c
 * Memory Manager */


#include "sys/alloc.h"
#include "sys/routix.h"




//typedef union header Header;

static Header base;
static Header *freep=NULL;


void *malloc (unsigned nbytes)
{
    Header *p, *prevp;
    unsigned nunits;
    nunits = (nbytes + sizeof(Header)-1) / sizeof(Header) +1;
    if ((prevp = freep)==NULL)
    {
	base.s.ptr = freep = prevp = &base;
	base.s.size = 0;
    }

    for ( p=prevp->s.ptr ;   ;  prevp=p , p=p->s.ptr)
    {
        if (p->s.size >= nunits)
	{
	
	    if (p->s.size == nunits)
		prevp->s.ptr = p->s.ptr;
	    else
	    {
		p->s.size -= nunits;
		p += p->s.size;
		p->s.size = nunits;
	    }
	    freep = prevp;
	    return (p+1);
	}
	else if (p==freep) {
	    if ( (p = morecore()) == NULL ) 
		return NULL;
//	    kprintf("\nMorecore entrego: %x\n", p);
	}
    }
	
}




Header *morecore(void)
{
    Header *up;

    up = (Header *) malloc_page();
    if (up == NULL)
    {
//	printf("Morecore: malloc_page sin memoria\n");
        return NULL;
    }
    printf("Morecore entrego: %x\n", up);

//    up->s.size= (PAGINA_SIZE + sizeof(Header)-1) / sizeof(Header) +1;
    
    up->s.size= PAGINA_SIZE / sizeof(Header);
    free ( (void *) (up+1));
    return freep;
}

void free (void *ap)
{
    Header *bp, *p;

    bp = (Header *) ap - 1;
    for (p=freep; ! (bp > p && bp < p->s.ptr) ; p=p->s.ptr)
        if ( (p >= p->s.ptr)  &&  (bp > p || bp < p->s.ptr))
	    break;
    if ( (bp + bp->s.size) == p->s.ptr)
    {
	bp->s.size += p->s.ptr->s.size;
	bp->s.ptr = p->s.ptr->s.ptr;
    }
    else bp->s.ptr = p->s.ptr;

    if ( (p+p->s.size) == bp)
    {
	p->s.size += bp->s.size;
	p->s.ptr = bp->s.ptr;
    }
    else p->s.ptr = bp;

    freep = p;    

}




