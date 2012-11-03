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

#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <process.h>
#include <errno.h>
#include <dir.h>
#include <stdarg.h>

#include "global.h"
#include "scriplib.h"
#include "port.h"
#include "sersetup.h"
#include "sermodem.h"

char *CONFIG = "SETUP.ROT";
char *ROTT   = "ROTT.ROT";
char *ApogeePath = "APOGEECD";

//******************************************************************************
//
// GetPathFromEnvironment ()
//
//******************************************************************************

void GetPathFromEnvironment( char *fullname, const char *envname, const char *filename )
   {
   char *path;

   path = getenv( envname );

   if ( path != NULL )
      {
      strcpy( fullname, path );
      if ( fullname[ strlen( fullname ) ] != '\\' )
         {
         strcat( fullname, "\\" );
         }
      strcat( fullname, filename );
      }
   else
      {
      strcpy( fullname, filename );
      }
   }

//****************************************************************************
//
// CheckParameter ()
//
//****************************************************************************

void CheckParameter (const char * s1, const char * file)
{
   if (strcmpi (token, s1))
      Error ("Could not find %s token in %s.\n",s1,file);
}

void ReadSetup (void)
{
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, CONFIG );
   if (access (filename, 0) == 0)
   {
      LoadScriptFile (filename);

      // Skip MODEMNAME
      GetToken (true);

      // Skip MODEMNAME string
      GetTokenEOL (false);


      // Get Initstring

      GetToken (true);

      CheckParameter ("MODEMINITSTR", filename);

      GetTokenEOL (false);

      if (name[0] != '~')
         strcpy (&initstring[0], &name[0]);
      else if (usemodem==true)
         Error("No INITSTRING defined in SETUP.ROT please run SETUP.EXE\n");
      else
         strcpy (&initstring[0], "ATZ\0");

      // Get Hangupstring

      GetToken (true);

      CheckParameter ("MODEMHANGUP", filename);

      GetTokenEOL (false);

      if (name[0] != '~')
         strcpy (&hangupstring[0], &name[0]);
      else if (usemodem==true)
         Error("No HANGUPSTRING defined in SETUP.ROT please run SETUP.EXE\n");
      else
         strcpy (&hangupstring[0], "ATZ\0");

      GetToken (true);

      CheckParameter ("BAUDRATE", filename);

      GetToken (false);

      baudrate = atol (token);

      GetToken (true);

      CheckParameter ("COMPORT", filename);

      GetToken (false);

      if (strcmpi (token, "~"))
         comport = atoi (token);
      else
         comport = -1;

      GetToken (true);

      CheckParameter ("IRQ", filename);

      GetToken (false);

      if (strcmpi (token, "~"))
         irq = atoi (token);
      else
         irq = -1;

      GetToken (true);

      CheckParameter ("UART", filename);

      GetToken (false);

      if (strcmpi (token, "~"))
         sscanf(token,"%x", &uart);
      else
         uart = -1;

      // Get Pulse parameter

      GetToken (true);

      CheckParameter ("PULSE", filename);

      GetToken (false);

      if (!(strcmpi (token, "YES")))
         pulse=true;
      else
         pulse=false;

      free (scriptbuffer);
   }
   else
   {
      Error ("Cannot find %s Please run SETUP.EXE\n",filename);
   }
   GetPathFromEnvironment( filename, ApogeePath, ROTT );
   if (access (filename, 0) == 0)
      {
      LoadScriptFile (filename);
      GetToken (true);
      CheckParameter ("PHONENUMBER", filename);
      GetTokenEOL (false);

      if (name[0] != '~')
         strcpy (&dialstring[0], &name[0]);
      else
         dialstring[0]=0;
      free (scriptbuffer);
      }
   else
   {
      Error ("Cannot find %s Please run SETUP.EXE\n",filename);
   }

}
