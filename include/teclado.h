/* teclado.h */

#define TECLADO_PORT 0x60

#define ALT	    0x0100
#define CTRL	    0x0200
#define SHIFT	    0x0300
#define TECLA_MODIFICADORA  0xff00

#define CARACTER_LIMITE	0x0400

#define HOME		0
#define CURSOR_UP	0
#define CURSOR_DOWN	0
#define CURSOR_LEFT	0
#define CURSOR_RIGHT	0

#define PAGE_UP		0
#define PAGE_DOWN	0
#define INSERT		0
#define DELETE		0
#define END		0
#define ENTER		0

#define F1	    0x1000
#define F2	    (F1 + 1)
#define F3	    (F1 + 2)
#define F4	    (F1 + 3)
#define F5	    (F1 + 4)
#define F6	    (F1 + 5)
#define F7	    (F1 + 6)
#define F8	    (F1 + 7)
#define F9	    (F1 + 8)
#define F10	    (F1 + 9)
#define F11	    (F1 + 10)
#define F12	    (F1 + 11)

#define CAPS_LOCK   0xD000
#define NUM_LOCK    0xE000
#define SCR_LOCK    0xF000

/* Obtiene el ascii de un scancode (en crudo) */
inline unsigned int _getascii (unsigned char code);

/* Obtiene el ascii de un scancode, teniendo en cuenta los modificadores (CAPS_LOCK, SHIFT, CTRL, etc) */
unsigned int getascii (unsigned char code);

/* Tabla de caracteres ASCII de un teclado standar */
unsigned int keymap_std[] = 
{
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 225, 39, '\b',
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 129, '+', '\n',
    CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 148, 132, '^', SHIFT, '#',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', SHIFT,
    '*', ALT, ' ', CAPS_LOCK, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUM_LOCK, SCR_LOCK, HOME, CURSOR_UP, PAGE_UP, '-',
    CURSOR_LEFT, 0, CURSOR_RIGHT, '+', END, CURSOR_DOWN, PAGE_DOWN, INSERT, DELETE, ENTER, 0, '<', F11, F12
};

/* Tabla de caracteres ASCII (con shift presionado) de un teclado standar */
unsigned int keymap_std_shift[] = 
{
   0, 0, '!', '"', '#', '$', '%', '&', '/', '(', ')', '=', 225, 39, '\b',
   0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 129, '+', '\n',
   CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 148, 132, '^', SHIFT, '#',
   'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', SHIFT,
   '*', ALT, ' ', CAPS_LOCK, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUM_LOCK, SCR_LOCK, HOME, CURSOR_UP, PAGE_UP, '-',
   CURSOR_LEFT, 0, CURSOR_RIGHT, '+', END, CURSOR_DOWN, PAGE_DOWN, INSERT, DELETE, ENTER, 0, '<', F11, F12
};



