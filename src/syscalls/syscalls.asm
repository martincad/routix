; Para las llamadas al sistema en el registro EAX se colocara, el codigo del grupo de llamadas en los dos bytes
; mas altos, y la funcion (dentro del grupo correspondiente) en los dos bytes mas bajos.
;
; Por ejemplo:
;  __asm__ __volatile__ ("movl %0, %%eax ; movl %1, %%ebx" : : "g" (SYS_PROCESS | SYS_EXEC) , "g" (tarea) : "eax" , "ebx");
;  __asm__ __volatile__ ("int $0x50");
;
; donde SYS_PROCESS es el grupo de llamadas al sistema relaciondas con el manejo de procesos
; y SYS_EXEC es la funcion dentro de SYS_PROCESS que ejecuta una tarea.
;
;

segment .text
[global _intSysCall]

[global _sys_process]
[global _sys_console]
[global _sys_timer]
[global _sys_mem]


[extern _syscall]
[extern _syscall_group_vector]	;array de punteros a grupos de funciones
[extern _syscall_group_max]		;array con cantidad maxima de llamadas al sistema por gupo
[extern _sys_no_existe]

[extern _KERNEL_PDT]	    ; Valor del CR3 del Kernel
[extern _actual]


MAX_SYSCALLS_GROUPS	EQU	5
MAX_SYSCALLS		EQU	10

;Aca me trae la interrupcion 0x50
_intSysCall:

	push ebx
	push eax

	shr eax, 16	;Verificar si el numero de grupo existe

	cmp eax, MAX_SYSCALLS_GROUPS
	ja  error_syscall_no

	mov ebx, [_syscall_group_max + eax * 4]		;Recuperar la cantidad maxima de llamadas que posee ese grupo

	pop eax

	;En ax tengo el numero de indice dentro del grupo. En bx el numero total de llamadas para ese grupo
	cmp ax, bx			
	pop ebx
	;Si intento llamar a una mayor que la "mayor", ERROR
	ja error_syscall_no

	;Pusheo los registros donde recibo los parametros   
	push ebp
	push esi
	push edi
	push edx
	push ecx
	push ebx

	;Usar como directorio de paginas el CR3 del Kernel
	mov ebx, [ _KERNEL_PDT ]
	mov cr3, ebx
	
	mov ebx, eax
	shr ebx, 16	;Pongo en ebx el numero de grupo de llamadas

	jmp [_syscall_group_vector + ebx * 4] ;El vector de grupos de llamadas a funciones se encuentra definido 
					      ;en syscalls.c


;Llamada al sistema no existe
error_syscall_no:

	push eax	
	call _sys_no_existe
	pop eax
	mov eax, -1
	iret


;Entrada al grupo de manejo de procesos
[extern _syscall_process]
_sys_process:

	and eax, 0xffff
	call [_syscall_process + eax * 4]

	jmp syscall_fin


;Entrada a manejo de teclado y video
[extern _syscall_console]
_sys_console:

	and eax, 0xffff
	call [_syscall_console + eax * 4]
	jmp syscall_fin


;Entrada a manejo de llamadas a sistema de timers
[extern _syscall_timer]
_sys_timer:
	
	and eax, 0xffff
	call [_syscall_timer + eax * 4 ]
	jmp syscall_fin

;Entrada a manejo de llamadas a sistema de memoria
[extern _syscall_mem]
_sys_mem:
	
	and eax, 0xffff
	call [_syscall_mem + eax * 4 ]
	jmp syscall_fin



; Balanceo el stack, reacomodo el CR3 y actual->cr3 con el valor de actual->cr3_backup
syscall_fin:

	add esp, 24

	mov ebx, dword [ _actual ]
	add ebx, 8
	mov ecx, dword [ ebx ]
	mov cr3, ecx
	
	mov ebx, dword [ _actual ]
	add ebx, 4
	mov dword [ ebx ], ecx

	iret

