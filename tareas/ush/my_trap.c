#include <signal.h>
char cmd[100];

main(int argc, char *argv[])
{
   void sighandler();
   int i, offset = 0;

   signal(SIGQUIT, sighandler);
   for (i = 1; i < argc; i++) {
       strcpy(cmd + offset, argv[i]);
       offset += strlen(argv[i]);
       cmd[offset++] = ' ';
   }
   cmd[offset] = '\0';
   pause();
}

void sighandler()
{
  system(cmd);
}
