[extern _flag_fork]

[global _jmp_tss]
[global _jmp_scheduler]

segment .text

;_jmp_scheduler:
;	jmp dword 0x28:0
;	ret

;_jmp_tss:

;	jmp dword 0x30:0
;	ret



; _actual apunta a la estructura de tipo task_struct_t del proceso actual, recordemosla:
;
;typedef struct task_struct_t
;{
;    dword esp0;
;    dword cr3;
;    dword cr3_backup;
;    pid_t pid;
;    struct task_struct_t *proxima;
;    char descripcion[MAX_DESCRIPTION];
;    byte estado;
;    word prioridad;
;    word cuenta;
;    dword tiempo_cpu;
;    struct file *open_files [MAX_FILES_POR_TAREA];  //definido en file.h
;    void *paginas[MAX_PAGINAS_POR_TAREA];
;    word num_code;  //cantidad de paginas de codigo de la tarea
;    word num_data;  //cantidad de paginas de datos y bss
;    int err_no;
;    signal_struct_t senales;
;    dword esp3;	//TEMP provisoria
;} task_struct_t

[extern _actual]

[extern _tss]

[extern _timertick_handler]
;[extern _senales_pendientes]

[global _switch_to_kernel_mode]
[global _entrada_de_inicio]

%define OFFSET_TSS_ESP0		4
%define KERNEL_STACK_TOP	0xd8000000
%define OFFSET_CR3		4

_switch_to_kernel_mode:	    ;(punto de entrada de la IRQ0)

	nop
	nop
	nop
	push ebp
	push esi
	push edi
	push edx
	push ecx
	push ebx
	push eax
	push ds
	push es
	push fs
	push gs


	; Guardamos el valor del esp de modo kernel de la tarea
	mov ebx, dword [ _actual ]
	mov dword [ ebx ], esp

	; Guardamos en la task_struct el valor del CR3
	mov ebx, dword [ _actual ]
	mov eax, cr3
	mov dword [ ebx + OFFSET_CR3 ], eax


	; Recuperamos el stack del kernel, así trabajamos tranquilos y sín límites
	mov eax, KERNEL_STACK_TOP
	mov esp, eax

	; Pasamos el control al kernel
	call _timertick_handler

_entrada_de_inicio:
	; Volvimos, ahora recuperamos el stack de modo kernel de la tarea
	; que puede ser diferente a la anterior, todo depende de si se venció
	; el tiempo de permanencia y/o el scheduler la switcheo (por medio del puntero _p)
	mov ebx, dword [ _actual ]
	mov eax, dword [ ebx ]
	mov esp, eax

	; CODIGO DE SENALES... empieza aca, todavia no termine nada !!!
	; Veamos si tenemos señales pendientes
	;push _actual
	;call _senales_pendientes
	;add esp, 4

	; Tambien acá recuperamos el CR3
	mov ebx, dword [ _actual ]
	mov eax, dword [ ebx + OFFSET_CR3 ]
	mov cr3, eax

	

	; Pisamos el valor de esp0 en el TSS para que la próxima interrupción pueda producirse
	; el cambio de modo ( usuario -> kernel ) de manera correcta

	; calculamos el top del esp0, recordando que estamos usando una página para el descriptor del proceso
	; y el stack, el cual encuentra su top al final del mismo, o sea:
	; top_esp0 = ( esp0 & 0xffffff00 ) + 4096

	mov eax, esp
	and eax, 0xfffff000
	add eax, 0x1000
	
	mov dword [ _tss + OFFSET_TSS_ESP0 ], eax

	; Recuperamos el contenido de todos los registros
	pop gs
	pop fs
	pop es
	pop ds
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop edi
	pop esi
	pop ebp

	iret

; Esta función tiene como fin atender los pedidos de interrupción (0x51), esta interrupción
; es utilizada por el kernel para switchear procesos cuando un proceso se pone a dormir y libera
; la CPU (ejemplo sys_usleep).
[global _intProximaTarea]
[extern _scheduler]

_intProximaTarea:

	nop
	nop
	nop
	push ebp
	push esi
	push edi
	push edx
	push ecx
	push ebx
	push eax
	push ds
	push es
	push fs
	push gs


	; Guardamos el valor del esp de modo kernel de la tarea
	mov ebx, dword [ _actual ]
	mov dword [ ebx ], esp

	; Guardamos en la task_struct el valor del CR3
	mov ebx, dword [ _actual ]
	mov eax, cr3
	mov dword [ ebx + OFFSET_CR3 ], eax


	; Recuperamos el stack del kernel, así trabajamos tranquilos y sín límites
	mov eax, KERNEL_STACK_TOP
	mov esp, eax

	; Pasamos el control al scheduler para buscar la proxima tarea a ejecutar
	call _scheduler

	; Volvimos, ahora recuperamos el stack de modo kernel de la tarea
	; que puede ser diferente a la anterior, todo depende de si se venció
	; el tiempo de permanencia y/o el scheduler la switcheo (por medio del puntero _p)
	mov ebx, dword [ _actual ]
	mov eax, dword [ ebx ]
	mov esp, eax

	; CODIGO DE SENALES... empieza aca, todavia no termine nada !!!
	; Veamos si tenemos señales pendientes
	;push _actual
	;call _senales_pendientes
	;add esp, 4

	; Tambien acá recuperamos el CR3
	mov ebx, dword [ _actual ]
	mov eax, dword [ ebx + OFFSET_CR3 ]
	mov cr3, eax

	

	; Pisamos el valor de esp0 en el TSS para que la próxima interrupción pueda producirse
	; el cambio de modo ( usuario -> kernel ) de manera correcta

	; calculamos el top del esp0, recordando que estamos usando una página para el descriptor del proceso
	; y el stack, el cual encuentra su top al final del mismo, o sea:
	; top_esp0 = ( esp0 & 0xffffff00 ) + 4096

	mov eax, esp
	and eax, 0xfffff000
	add eax, 0x1000
	
	mov dword [ _tss + OFFSET_TSS_ESP0 ], eax

	; Recuperamos el contenido de todos los registros
	pop gs
	pop fs
	pop es
	pop ds
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop edi
	pop esi
	pop ebp

	iret

