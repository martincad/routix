
int putchar ( int );
int puts ( const char *);
byte *nextpos ( byte *);
void clrscr (void);
void scroll_up ( void );
unsigned char *gotoxy ( int , int );
void kprintf ( char *string, ... );
char getascii_v ( char );	// Getascii para video
void printn( unsigned int num, int base);

unsigned char inportb (word puerto);
void outportb (word puerto, unsigned char dato);

unsigned char inportb_p (word puerto);
void outportb_p (word puerto, unsigned char dato);
