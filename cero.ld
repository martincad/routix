OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS {

 .text 0xC0000000 :
{
	obj/boot.o(.text)
	obj/*.o(.text)
	obj/*.a(.text)
}

__code_size = SIZEOF (.text);

 .data 0xC8000000 : AT (ADDR (.text) + SIZEOF (.text) )
{
	obj/*(.data)
}

__data_size = SIZEOF (.data);

.bss  : 
{
	obj/*(.bss)
}


} 
