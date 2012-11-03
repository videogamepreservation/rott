/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <conio.h>
#include <dos.h>


/* QUESIZE must be a power of 2 */

#define  QUESIZE 2048

typedef struct
{
	int	head, tail;		// bytes are put on head and pulled from tail
	int	size;
	unsigned char	data[QUESIZE];
} que_t;

extern	que_t		inque, outque;
extern int irq;
extern int uart;
extern int comport;
extern long baudrate;

#define QueSpot(index)  (((index) & (QUESIZE-1)))

void jump_start( void );
void interrupt isr_8250 (void);
void interrupt isr_16550 (void);
boolean Is8250 ( void );
void GetUart ( void );
void InitPort ( void );
void ShutdownPort ( void );
int  read_byte( void );
void write_byte( unsigned char c );
void write_bytes( char *buf, int count );
void write_buffer( char *buffer, unsigned int count );
