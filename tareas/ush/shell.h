#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0
#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w) >>8)

typedef short BOOLEAN;

#define MAXVAR 50

static struct varslot {
     char *name;
     char *val;
     BOOLEAN  exported;
   }sym[MAXVAR];

extern char **environ;

char *malloc(), *realloc();

static BOOLEAN assign(char **p, char *s)  /* initialize name or value */
{
    int size;
 
    size = strlen(s) +1;
    if (*p == NULL) {
       if(( *p = malloc(size)) == NULL)
          return(FALSE);
     }
    else if (( *p = realloc(*p, size)) == NULL)
         return(FALSE);
    strcpy (*p, s);
    return(TRUE);
}
 

BOOLEAN EVexport(char *name)  /*set variable to be exported */
{
    struct varslot *v;
    static struct varslot *find( );

    if ( (v=find(name) ) == NULL)
          return(FALSE);
    if (v->name == NULL)
         if(!assign(&v->name, name) ||!assign(&v->val, ""))
          return(FALSE);
    v->exported = TRUE;
    return(TRUE);
  }


static struct varslot *find(char *name)  /*find symbol table entry*/
{

   int i;
   struct varslot *v;

   v = NULL;
   for(i=0;i<MAXVAR;i++)
      if( sym[i].name == NULL){
          if ( v == NULL) v = &sym[i];
	}
      else if (strcmp(sym[i].name, name) == 0){
               v = &sym[i];
               break;
	     }
    return(v);
 }

BOOLEAN EVset(char *name, char *val)/*add name & valude to enviromnemt*/
{
    struct varslot *v;

    if (( v=find(name)) == NULL)
         return(FALSE);
    return(assign(&v ->name, name)&& assign(&v->val, val));
  }


BOOLEAN EVinit() /* initialize symbol table from environment */
{
    int i ,namelen;
    char name[100];

    for (i=0; environ[i] != NULL; i++){
       namelen = strcspn(environ[i] ,"=");
       strncpy(name,environ[i],namelen);
       name[namelen ]='\0';
       if (!EVset (name,&environ[i][namelen +1])|| !EVexport (name))
           return(FALSE);
    }
    return(TRUE);
}


char *EVget(char *name) /* get value of variable */
{
   struct varslot *v;
  
   if (( v=find(name) ) == NULL || v->name ==NULL)
         return(NULL);
   return( v->val);
}

void fatal(char *msg)   /* print wrror message and terminate */
{
   fprintf(stderr, "ERROR:%s\n", msg);
   exit(1);
 }

void syserr (char *msg)/* print system callerror message and terminate*/
{
   extern int errno, sys_nerr;
   extern char *sys_errlist[];
  
   fprintf(stderr, "ERROR: %s (%d", msg,errno);
   if ( errno > 0 && errno < sys_nerr )
      fprintf(stderr, ";%s)\n",sys_errlist[errno]);
   else
      fprintf(stderr, ")\n");
   exit(1);
 }


BOOLEAN EVupdate() /* build environment from symbol table */
{
     int i, envi, nvlen;
     struct varslot *v;
     static BOOLEAN updated = FALSE;

     if ( !updated)
        if((environ = (char **)malloc((MAXVAR +1) *sizeof(char *)))
              == NULL)
            return(FALSE);
     envi =0;
     for(i=0; i<MAXVAR;i++){
         v = &sym[i];
         if (v->name ==NULL || !v->exported)
           continue;
         nvlen = strlen(v->name) + strlen(v->val) +2;
         if(!updated){
            if ((environ[envi] = malloc(nvlen)) ==NULL)
                return(FALSE);
          }
         else if( (environ[envi] = realloc(environ[envi],nvlen)) ==NULL)
                return(FALSE);
         sprintf(environ[envi], "%s=%s", v->name, v->val);
         envi++;
     }
     environ[envi] = NULL;
     updated = TRUE;
     return(TRUE);
}
 


void EVprint()  /* printf environment */
{
  int i;
 
  for(i=0;i<MAXVAR;i++)
      if(sym[i].name !=NULL)
        printf("%3s %s=%s\n", sym[i].exported?"[E]":"",
              sym[i].name, sym[i].val);
}

void asg(int argc,char *argv[]) /* assignment command*/
{

   char *name, *val, *strtok();

   if (argc != 1)
       printf("Extra args\n");
   else{
       name = strtok(argv[0], "=");
       val = strtok(NULL, "\1");  /* get all that's left*/
       if (!EVset(name, val))
          printf("Cannot assign\n");
     }
 }

void set (int argc,char *argv[] ) /*set command */
{
   if( argc!=1)
     printf("Extra args\n");
   else
     EVprint();
 }


void export(int argc,char *argv[])  /*export command */
{
  int i;
  
  if (argc ==1){
    set(argc, argv);
    return;
  }
  for(i=1; i<argc;i++)
    if(!EVexport (argv[i])) {
        printf("Cannot export %s\n", argv[i]);
        return;
      }
}


void statusprt (int pid,int status) /* interrupt status code */
{
     int code;
     static char *sigmsg[] = {
        "",
        "Hangup",
        "Interrupt",
        "Quit",
        "Illegal instruction",
        "Trace trap",
        "IOT instruction",
        "EMT instruction",
        "Floating point exception",
        "Kill",
        "Bus error",
        "Segmentation violation",
        "Bad arg to system call",
        "Write on pipe",
        "Alarm clock",
        "Terminate signal", 
        "User signal 1",
        "User signal 2",
        "Death of child",
        "Power fail",
      };
     
    if (status != 0 && pid != 0)
           printf("Process %d :", pid);
    if (lowbyte(status) == 0){
           if (( code = highbyte (status)) !=0)
               printf("Exit code %d\n", code);
	 }
    else{
        if((code = status & 00177) <=MAXSIG)
           printf("%s", sigmsg[code]);
        else 
           printf("Signal #%d", code);
        if (( status & 0200) == 0200)
             printf("-core dumped");
        printf("\n");
      }
   }


static void (*entryint)(), (*entryquit)();

void ignoresig() /* ignore interrupt and quit  */
{
   static BOOLEAN first = TRUE;

   if(first) {
        first = FALSE;
        entryint = signal(SIGINT, SIG_IGN);
        entryquit = signal(SIGQUIT, SIG_IGN);
        if( entryint == SIG_ERR || entryquit == SIG_ERR )
            syserr("signal");
      }

    else if (signal(SIGINT, SIG_IGN) ==  SIG_ERR ||  
          signal(SIGQUIT, SIG_IGN) == SIG_ERR)
               syserr("signal");
   }

void entrysig() /* restore interrupt and quit  */
{ 
     if(signal (SIGINT, entryint) == SIG_ERR || 
        signal(SIGQUIT, entryquit) == SIG_ERR)
            syserr("signal");
   }

void catchsigs () /* catch signals  */
{
     void cleanup();
     static void setsig( );

     setsig(SIGHUP, cleanup);
     setsig(SIGINT, cleanup);
     setsig(SIGQUIT,cleanup);
     setsig(SIGTERM,cleanup);
   }

static void setsig(sig, fcn) /* set signal  if defaulted */
int sig;
void (*fcn)();
{
    if(signal(sig, SIG_IGN) == SIG_ERR) syserr("signal");
    else if(signal(sig,SIG_IGN)== SIG_IGN) return;
    else if(signal(sig,SIG_IGN)==  SIG_DFL){
          if( signal(sig, fcn)==SIG_ERR)syserr("signal");
	}
    else
         fatal("signal already caught!");
   
 }

void cleanup(int sig)  /* clean up and terminate */
{
    
    if( signal(sig, SIG_IGN) == SIG_ERR)
        syserr("signal");
 
    switch (sig) {
    case SIGHUP:
         fprintf(stderr, "Hangup.\n");
         break;
    case SIGINT:
         fprintf(stderr, "Interrupt.\n");
         break;
    case SIGQUIT:
         fprintf(stderr, "Quit.\n");
    }
    exit(1);
}           

typedef enum { T_WORD, T_BAR,T_AMP, T_SEMI, T_GT, T_GTGT, T_LT,
                         T_NL, T_EOF } TOKEN;
static TOKEN gettoken (char *word)
{
   enum { NEUTRAL, GTGT ,INQUOTE, INWORD } state = NEUTRAL;
   int c;
   char  *w;

   w = word;
   while (( c=getchar ())!=EOF){
     switch (state) {
       case NEUTRAL:
       switch (c){
              case ';':
                 return(T_SEMI); 
              case '&':
                 return(T_AMP); 
              case '|':
                 return(T_BAR); 
              case '<':
                 return(T_LT); 
              case '\n':
                 return(T_NL); 
	      case ' ':
              case '\t':
                 continue; 
              case '>':
                 state=GTGT;
                 continue;
              case '"':
                 state=INQUOTE;
                 continue; 
              default:
                 state=INWORD;
                 *w++=c;
                 continue; 
       }
       case  GTGT:
            if (c== '>')
               return(T_GTGT);
            ungetc(c, stdin); 
            return(T_GT);
       case INQUOTE:
            switch (c){
            case '\\':
            *w++=getchar();
            continue; 
            case '"':
            *w =  '\0';
            return(T_WORD);
            default:
            *w++=c;
            continue; 
   }
       case  INWORD:
           switch (c){
             case '\\':
             case '&':
             case '|':
             case '<':
             case '>':
             case '\n':
             case ' ':
             case '\t':
             ungetc(c,stdin);
             *w='\0';
             return (T_WORD);
             default:
             *w++ =c;
             continue; 
	   }
     }
   }
       return (T_EOF);
}
