; int.asm

segment .text
use32

[global _intDefaultHandler]
[global _intTeclado]
[global _intFloppy]
[global _intSysCall]

;[global _int_timertick_handler]
[extern _DefaultHandler]
[extern _Teclado]
;[extern _timertick_handler]
[extern _ExcepcionHandler]
[extern _Floppy]
[extern _syscall]

%macro excepcion 1
[global _excepcion%1]
_excepcion%1:
	push dword %1
	call _ExcepcionHandler
	add esp, 4
	iret
%endmacro

%macro SAVE_ALL 0
	push eax
	push ebx
	push ecx
	push edx
	push ebp
	push esi
	push edi
%endmacro 

%macro RESTORE_ALL 0
	pop edi
	pop esi
	pop ebp
	pop edx
	pop ecx
	pop ebx
	pop eax
%endmacro


%assign i 0
%rep 20 
excepcion i
%assign i i+1
%endrep



_intDefaultHandler:
	SAVE_ALL
	call _DefaultHandler
	RESTORE_ALL
	iret


;_int_timertick_handler:
;	SAVE_ALL
;	call _timertick_handler
;	RESTORE_ALL
;	iret

_intTeclado:
	SAVE_ALL
	call _Teclado
	RESTORE_ALL
	iret


_intFloppy:
	SAVE_ALL
	call _Floppy
	RESTORE_ALL
	iret

