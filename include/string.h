/* string.h */

int strcmp(const char *s1, const char *s2);
char *strcpy (char *dest, const char *src); 
char *strcat(char *dest, char *src);
size_t strlen(const char *s);
char *strncpy (char *dest, const char *src, size_t nbytes); 

char *str_to_upper (char *str);
char *str_to_lower (char *str);
int tolower(int c);
int toupper(int c);

int memcmp ( const void *s1, const void *s2, dword n);
void *memcpy ( void *dest, const void *src, dword n);


