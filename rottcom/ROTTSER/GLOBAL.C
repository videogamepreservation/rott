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
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <dir.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "sersetup.h"
#include "scriplib.h"

/*
=================
=
= Error
=
= For abnormal program terminations
=
=================
*/

void Error (char *error, ...)
{
	va_list argptr;


	if (error)
	{
		va_start (argptr,error);
		vprintf (error,argptr);
		va_end (argptr);
		printf ("\n\n");
      ShutDown();
	}

	exit (error != (char *) NULL);
}

/*
=================
=
= CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc-1) or 0 if not present
=
=================
*/

int CheckParm (char *check)
{
	int             i;

	for (i = 1;i<_argc;i++)
		if ( !stricmp(check,_argv[i]) )
			return i;

	return 0;
}


//******************************************************************************
//
// SafeOpenWrite ()
//
//******************************************************************************

int SafeOpenWrite (char *filename)
{
	int	handle;

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}

//******************************************************************************
//
// SafeOpenRead ()
//
//******************************************************************************

int SafeOpenRead (char *filename)
{
	int	handle;

	handle = open(filename,O_RDONLY | O_BINARY);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}


//******************************************************************************
//
// SafeRead ()
//
//******************************************************************************

void SafeRead (int handle, void *buffer, long count)
{
	long iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (read (handle,buffer,(int)iocount) != iocount)
			Error ("File read failure");
		buffer = (void *)( (byte *)buffer + (int)iocount );
		count -= iocount;
	}
}


//******************************************************************************
//
// SafeWrite ()
//
//******************************************************************************

void SafeWrite (int handle, void *buffer, long count)
{
	long	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,(int)iocount) != iocount)
			Error ("File write failure");
		buffer = (void *)( (byte *)buffer + (int)iocount );
		count -= iocount;
	}
}

//******************************************************************************
//
// SafeWriteString ()
//
//******************************************************************************

void SafeWriteString (int handle, char * buffer)
{
	unsigned	iocount;

   iocount=strlen(buffer);
	if (write (handle,buffer,iocount) != iocount)
			Error ("File write string failure");
}

//******************************************************************************
//
// SafeMalloc ()
//
//******************************************************************************

void *SafeMalloc (long size)
{
	void *ptr;

	ptr = malloc ((short)size);

	if (!ptr)
		Error ("Malloc failure for %lu bytes",size);

	return ptr;
}


//******************************************************************************
//
// LoadFile ()
//
//******************************************************************************

long	LoadFile (char *filename, void **bufferptr)
{
   int  handle;
   long length;

   handle = SafeOpenRead (filename);
   length = filelength (handle);
   *bufferptr = SafeMalloc(length);
   SafeRead (handle,*bufferptr, length);
   close (handle);
   return length;
}


//******************************************************************************
//
// ReadParameter
//
//******************************************************************************

void ReadParameter (const char * s1, int * val)
{
   GetToken (true);
   if (endofscript)
      Error("Could not find %s as a parameter in configuration file\n",s1);
   while (1)
      {
      if (!strcmpi (token,s1))
         {
         GetToken(false);
         *val=atoi(token);
         return;
         }
      GetToken(false);
      if (endofscript)
         Error("Could not find %s as a parameter in configuration file\n",s1);
      }
}

//******************************************************************************
//
// WriteParameter
//
//******************************************************************************

void WriteParameter (int file, const char * s1, int val)
{
   char s[50];

   // Write out Header
   SafeWriteString (file, (char *)s1);

   // Write out space character
   strcpy (&s[0],(const char *)"  ");
   SafeWriteString (file, &s[0]);

   // Write out value
   itoa(val,&s[0],10);
   SafeWriteString (file, &s[0]);

   // Write out EOL character
   strcpy (&s[0],(const char *)"\n");
   SafeWriteString (file, &s[0]);
}


//******************************************************************************
//
// StringLength ()
//
//******************************************************************************

int StringLength (char *string)
{
	int length=0;

   while ((*string)!=0)
   {
      length++;
      string++;
   }

   length++;

   return (length);
}

//******************************************************************************
//
// UL_strcpy ()
//
//******************************************************************************

void UL_strcpy (byte * dest, byte * src, int size)
{
   byte *d;
   byte *s;
   int cnt = 0;

   d = dest;
   s = src;

   while (*s && (cnt < size))
   {
      *d++ = *s++;
      size ++;
   }
   *d = 0;
}

