OUTPUT_FORMAT(coff-go32)
ENTRY(_main)

SECTIONS {

 .text 0x80000000 :
{
	*.o(.text)
}

__code_size = SIZEOF (.text);

 .data 0x88000000 : AT (ADDR (.text) + SIZEOF (.text) )
{
	*.o(.data)
}

__data_size = SIZEOF (.data);

.bss  : 
{
	*.o(.bss)
}

.comment :
{
	*(.comment)
}

} 
