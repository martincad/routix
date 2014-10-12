routix
======

Routix is a POSIX compliant monolithic Kernel written more than 10 years ago in order to learn (trough hands-on) the i386 architecture and how an Operating System is written.

Most of the comments were done in Spanish ( sorry about that ).

The OS implements:
1. FAT 16 fs
1. Syscalls:
 1. Console:   
    - int sys_gets (char *str);
    - int sys_print (void *buff, size_t largo);
    - int sys_clrscr (void);
 2. Process:
    - int sys_fork (void);
    - int sys_void (void);
    - int sys_exec (char *nombre);
    - void sys_perror (char *str);
    - int sys_renice (word pid, word prioridad);
    - inline pid_t sys_get_pid (void);
    - inline pid_t sys_get_ppid (void);
    - void sys_exit (int);
    - void sys_show (int);
    - pid_t sys_wait(int *);
    - pid_t sys_waitpid (pid_t pid, int *status, int options);
 3. Timers
    - int sys_sleep(int segundos);
    - int sys_usleep(int usegundos);
    - int sys_proc_dump(void);
    - extern int sys_kill(pid_t pid, int sig);
    - int sys_proc_dump_v(int);
    - int sys_timer_dump(void);
 4. Memory Management
    - void *sys_malloc_page(void);
    - int sys_free_page(void *);
    - dword sys_free_mem (void);
 5. A small C library was implemented so it is possible to build a Routix executable with GCC linking against Routix lib C library.
The only executable format supported is COFF32
