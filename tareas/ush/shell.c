/*
   This partial code is a modified version of Marc Rochkind's A Real Shell,
   published in "Advanced UNIX Programming", Prentice Hall, 1985.
*/

#include "shell.h"

#define MAXARG 20
#define MAXFNAME 500
#define MAXWORD  100
#define BADFD -2

main()  /* real shell */
{
   char *prompt;
   int pid, fd;
   TOKEN  term; 
   static TOKEN command();
   static void waitfor();

   ignoresig();
   if(!EVinit())
        fatal ("can't initialize environment");
   if( (prompt = EVget("PS2")) == NULL)
        prompt = ">";
   printf("%s", prompt);

   while(1){
        term = command(&pid, FALSE, NULL);
        if ( term != T_AMP && pid != 0)
             waitfor(pid);
        if ( term == T_NL)
             printf("%s", prompt);
        for(fd = 3; fd< 20; fd++)
             (void)close(fd);  /*ignore error */
      }
}

static TOKEN 
command(int *waitpid, BOOLEAN  makepipe, int *pipefdp) /* do simple cmd */
{
  
     TOKEN token, term ;
     int argc, srcfd, dstfd, pid, pfd[2];
     char *argv[MAXARG + 1], srcfile[MAXFNAME],dstfile[MAXFNAME];
     char word[MAXWORD], *malloc();
     BOOLEAN append;
     static int invoke( );

     argc = 0;
     srcfd = 0;
     dstfd = 1;
     while (1) {
         switch ( token = gettoken(word)) {
         case T_WORD:
              if (argc == MAXARG){
                  fprintf(stderr, "Too many args\n");
                  break;
		}
              if (( argv[argc] = malloc(strlen(word) +1)) == NULL) {
                   fprintf(stderr, "Out of argmemory\n");
                   break;
		 }
              strcpy(argv[argc], word);
              argc ++;
              continue;
	 case T_LT:
               if(makepipe){
                  fprintf(stderr, "Extra<\n");
                  break;
                }
               if (gettoken(srcfile) != T_WORD){
                   fprintf(stderr, "Illegal <\n");
                   break;
		 }
               srcfd = BADFD;
               continue;
	 case T_GT:
               if (dstfd !=1) {
                   fprintf(stderr, "Extra > or >> \n");
                   break;
                 }
	       if (gettoken(dstfile) != T_WORD) {
                   fprintf(stderr, "Illegal > or >> \n");
                   break;
                 }
               dstfd = BADFD;
               append = FALSE;
               continue;
	 case T_GTGT:
               if (dstfd != 1) {
                   fprintf(stderr, "Extra > or >>\n"); 
                   break;
                }
               if (gettoken(dstfile) != T_WORD){
                   fprintf(stderr, "Illegal >or >>\n");
                   break;
		 }
               dstfd = BADFD;
               append = TRUE;
               continue;
	 case T_BAR:
         case T_AMP:
         case T_SEMI:
         case T_NL:
               argv[argc] = NULL;
               if ( token == T_BAR) {
                    if ( dstfd !=1) {
                         fprintf(stderr, "> or >> conflicts with |\n");
                         break;
		       }
                    term = command ( waitpid, TRUE, &dstfd);
		  }
               else 
                    term = token;
               
               if ( makepipe ){
                     if ( pipe(pfd) == -1)
                          syserr("pipe");
                          *pipefdp = pfd[1];
                          srcfd = pfd[0];
		 }
               if(term == T_AMP)pid = invoke(argc, argv, srcfd, 
                            srcfile, dstfd, dstfile, append, TRUE);
               else pid=invoke(argc,argv,srcfd, srcfile,dstfd, dstfile,
                           append, FALSE);

               if (token != T_BAR)
                    *waitpid = pid;
               if (argc == 0 && (token != T_NL || srcfd >1))
                    fprintf(stderr, "Missing command\n");
               while (--argc >= 0)
                      free(argv[argc]);
               return(term);
	     case T_EOF:
                  exit(0);
	     }  
       }
}

static int invoke(int argc,char *argv[],int srcfd, char *srcfile,
          int dstfd,char *dstfile,BOOLEAN append, BOOLEAN bckgrnd) 
                              /* invoke simple command */
{

    static void redirect();
    int pid;
    static BOOLEAN builtin();
  
    if (argc == 0  || builtin(argc, argv, srcfd, dstfd))
        return(0);
    switch (pid = fork()) {
    case -1:
         fprintf(stderr, "Can't create new process\n");
         return(0);
    case 0:
         if (!bckgrnd)
             entrysig();
         if (!EVupdate())
             fatal("can't update environment");
         redirect(srcfd, srcfile, dstfd, dstfile, append, bckgrnd);
         execvp(argv[0], argv);
         fprintf(stderr, "Can't execute %s\n", argv[0]);
         exit(0);
    default:
         if (srcfd > 0 && close(srcfd) == -1)
             syserr("close src");
         if (dstfd > 1 && close(dstfd) == -1)
             syserr("close dst");
         if (bckgrnd)
             printf("%d\n", pid);
         return(pid);
    }
}
/*     REDIRECCION ANULADA MOMENTANEAMENTE
static void redirect(int srcfd, char *srcfile, int dstfd,char *dstfile,
	     BOOLEAN append, BOOLEAN bckgrnd)  /* I/O redirection */
{
   int flags, fd;
   long lseek();

   if (srcfd == 0 && bckgrnd) {
       strcpy(srcfile, "/dev/null");
       srcfd = BADFD;
    }
   
   if (srcfd !=0 ) {
       if ( close(0) == -1)
            syserr("close");
       if (srcfd > 0) {
            if ( dup(srcfd) !=0)
                 fatal("dup");
             }
       else if (open (srcfile, O_RDONLY, 0) == -1){
             fprintf(stderr, "Can't open %s\n", srcfile);
             exit(0);
       }
   }
 
   if (dstfd !=1) {
       if (close(1) == -1)
           syserr("close");
       if (dstfd > 1){
           if (dup(dstfd) != 1)
               fatal("dup");
	    }
       else {
            flags = O_WRONLY | O_CREAT;
            if(!append)
                flags |= O_TRUNC;
            if (open(dstfile, flags, 0666) == -1) {
               fprintf(stderr, "Can't create %s\n", dstfile);
               exit(0);
	     }
            if(append)
               if(lseek(1, 0L,2) == -1)
                 syserr("lseek");
	  }
     }
   for (fd =3; fd<20; fd++)
       (void)close(fd);  // ignore error
}
*/
static void waitfor(int pid)  /* wait for child */
{

    int wpid, status;

    while( (wpid = wait(&status)) != pid && wpid !=-1 )
         statusprt(wpid, status);
    if (wpid == pid )
         statusprt(0, status);
}

static BOOLEAN builtin(int argc, char *argv[], int srcfd, int  dstfd) 
                              /* do built-in */
{
     char *path;

     if (strchr(argv[0], '=') !=NULL)
         asg(argc, argv);
     else if (strcmp(argv[0], "export") == 0 )
          export(argc, argv);
     else if (strcmp(argv[0], "set") == 0 )
          set(argc, argv);
     else if (strcmp(argv[0], "cd") == 0 ){
          if ( argc >1)
              path = argv[1];
          else 
              if (( path = EVget("HOME")) ==NULL)
                    path = ".";
          if (chdir(path) ==-1)
              fprintf(stderr, "%s: bad directory\n", path);
	}
     else 
          return( FALSE);
     if (srcfd !=0 || dstfd !=1)
          fprintf(stderr, "Illegal redirection or pipeline\n");
}
