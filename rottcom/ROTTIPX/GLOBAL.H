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

#ifndef global_public
#define global_public

#define TRUE  1
#define FALSE 0
#define EOS '\0'
#define ESC 0x1B

#define CLOCK_FREQUENCY 1843200		/* 1.8432 Mhz */

#define INPUT( port )        inp( port )
#define OUTPUT( port, data ) (void) outp( port, data )
#define CLI()                disable()
#define STI()                enable()

typedef enum {false, true} boolean;
typedef unsigned char byte;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long LONG;


void Error (char *error, ...);
int CheckParm (char *check);

#endif
