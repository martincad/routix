/* video.h */

#define VIDEO_BASE   0xB8000

/* Registros del Controlador de Video (escritura)*/
#define PORT_VIDEO_REG_WR   0x3d4
#define PORT_VIDEO_VALUE_WR 0x3d5

/* Registros del Controlador de Video (de lectura)*/
#define PORT_VIDEO_REG_RD   0x3d4
#define PORT_VIDEO_VALUE_RD 0x3d5

#define VIDEO_LOCATION	    0xc
#define CURSOR_LOCATION	    0xe

/* Enviar el word "valor" al "registro" del 6845 */
#define setword_vga(registro,valor) {cli(); \
				     outportb (PORT_VIDEO_REG_WR, registro); \
				     outportb (PORT_VIDEO_VALUE_WR, (byte) (valor>>8) & 0xFF); \
			    	     outportb (PORT_VIDEO_REG_WR, registro + 1); \
				     outportb (PORT_VIDEO_VALUE_WR, (byte) valor ); \
				     sti(); }

/* Enviar el byte "valor" al "registro" del 6845 */
#define setbyte_vga(registro,valor) {cli(); \
				     outportb (PORT_VIDEO_REG_WR, registro); \
				     outportb (PORT_VIDEO_VALUE_WR, valor ); \
				     sti(); }				     

/* Ubica al cursor en la posicion indicada por "cursor_video" */
#define refresh_cursor() setword_vga(CURSOR_LOCATION, consola->posicion_cursor);

