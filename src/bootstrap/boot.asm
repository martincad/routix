CPU 386

segment .text
use32


[global _start]

_start:

	
[extern _init]
	
	mov ax, 0x10
	mov ss, ax
	mov esp, 0x200000

	jmp _init

