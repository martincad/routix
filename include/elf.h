/* elf.h */

#ifndef __SYSTEM
#include "../include/system.h"
#endif

typedef unsigned int Elf32_Addr;  
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef int Elf32_Sword;
typedef unsigned int Elf32_Word;

#define EI_NIDENT   16

typedef struct Elf32_Ehdr {
    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half	    e_type;
    Elf32_Half	    e_machine;
    Elf32_Word	    e_version;
    Elf32_Addr	    e_entry;
    Elf32_Off	    e_phoff;
    Elf32_Off	    e_shoff;
    Elf32_Word	    e_flags;
    Elf32_Half	    e_ehsize;
    Elf32_Half	    e_phentsize;
    Elf32_Half	    e_phnum;
    Elf32_Half	    e_shentsize;
    Elf32_Half	    e_shnum;
    Elf32_Half	    s_shstrndx;
}   Elf32_Ehdr;

//e_type
#define ET_NONE	    0
#define ET_REL	    1
#define	ET_EXEC	    2
#define	ET_DYN	    3
#define ET_CORE	    4
#define ET_LOPROC   0xff00
#define ET_HIPROC   0xffff

//e_version
#define EV_NONE	    0
#define EV_CURRENT  1

#define EM_386	    3


struct coff_header{
    unsigned short f_magic;         /* magic number             */
    unsigned short f_nscns;         /* number of sections       */
    unsigned long  f_timdat;        /* time & date stamp        */
    unsigned long  f_symptr;        /* file pointer to symtab   */
    unsigned long  f_nsyms;         /* number of symtab entries */
    unsigned short f_opthdr;        /* sizeof(optional hdr)     */
    unsigned short f_flags;         /* flags                    */
};

// Valor de f_magic para un COFF32
#define	COFF32_TYPE   0x14c

struct coff_sections{
    char           s_name[8];  /* section name                     */
    unsigned long  s_paddr;    /* physical address, aliased s_nlib */
    unsigned long  s_vaddr;    /* virtual address                  */
    unsigned long  s_size;     /* section size                     */
    unsigned long  s_scnptr;   /* file ptr to raw data for section */
    unsigned long  s_relptr;   /* file ptr to relocation           */
    unsigned long  s_lnnoptr;  /* file ptr to line numbers         */
    unsigned short s_nreloc;   /* number of relocation entries     */
    unsigned short s_nlnno;    /* number of line number entries    */
    unsigned long  s_flags;    /* flags                            */
} ;

#define COFF32_TEXT	0x20
#define COFF32_DATA	0x40
#define COFF32_BSS	0x80
    
