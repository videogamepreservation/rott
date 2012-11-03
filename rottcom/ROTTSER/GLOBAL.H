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
#ifndef _GLOBAL_
#define _GLOBAL_

#define TRUE  1
#define FALSE 0
#define EOS '\0'
#define ESC 0x1B

#define CLOCK_FREQUENCY 1843200L		/* 1.8432 Mhz */

#define INPUT( port )        inp( port )
#define OUTPUT( port, data ) (void) outp( port, data )
#define CLI()                disable()
#define STI()                enable()

#define ROTTSER 1
typedef enum {false, true} boolean;
typedef unsigned char byte;
typedef unsigned short int  word;
typedef unsigned long       longword;
typedef long                fixed;

void Error (char *error, ...);
int CheckParm (char *check);
int SafeOpenWrite (char *filename);
int SafeOpenRead (char *filename);
void SafeRead (int handle, void *buffer, long count);
void SafeWrite (int handle, void *buffer, long count);
void SafeWriteString (int handle, char * buffer);
void *SafeMalloc (long size);
long	LoadFile (char *filename, void **bufferptr);
void	SaveFile (char *filename, void *buffer, long count);
int US_CheckParm (char *parm, char **strings);
void ExtractFileBase (char *path, char *dest);
void UL_DisplayMemoryError (void);
void ReadParameter (const char * s1, int * val);
void WriteParameter (int file, const char * s1, int val);
int StringLength (char *string);
void UL_strcpy (byte * dest, byte * src, int size);

#endif
