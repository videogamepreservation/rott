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

// sersetup.c

#include "global.h"
#include "sersetup.h"
#include "sermodem.h"
#include "sercom.h"
#include "port.h"
#include "..\rottnet.h"
#include "st_cfg.h"
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <stdarg.h>
#include <bios.h>
#include <ctype.h>


#define VERSION "1.3"

boolean usemodem=false;
boolean showstats=false;

void ShutDown( void )
{
 	if (usemodem==true)
		hangup_modem ();

   ShutdownROTTCOM();

	ShutdownPort ();
}

void talk (void)
{
	int c;

	clrscr ();
	printf ("Talk mode - You might want to decide which type of COMM-BAT (tm)\n");
   printf ("            game you would like to play. Both players must\n");
   printf ("            press the <ESC> key to continue into the game.\n\n");
	while (TRUE)
	{
		/* Check for keypress */
		if (bioskey (1))
		{
			if ((c = bioskey (0) & 0xff) == ESC)
			{
				/* ESC key -- Exit talk mode -- Flush both sides */
//            write_buffer (&(char)c, 1);
				while (read_byte () != -1)
				;
				while (bioskey (1))
					bioskey (0);
				printf ("\n\n");
				return;
			}
			if (c == 0x0D)		/* Change cr to crlf */
				c = 0x0A;
			write_buffer (&(char)c, 1);
			printf ("%c", c);
		}
		/* Check for received byte */
		if ((c = read_byte ()) != -1)
         {
#if 0
         if (c==ESC)
            {
				while (read_byte () != -1)
				;
				while (bioskey (1))
					bioskey (0);
				printf ("\n\n");
				return;
            }
#endif
			printf ("%c", c);
         }
	}
}


/*
=================
=
= main
=
=================
*/

void main( void )
{
   boolean answer=false;

	clrscr ();
	printf("                     ----------------------------------\n");
   printf("                       ROTT SERIAL DEVICE DRIVER V%s \n",VERSION);
   printf("                     ----------------------------------\n");
//
// set network characteristics
//
	rottcom.numplayers = 2;
   pause=false;
	rottcom.gametype = MODEM_GAME;
	rottcom.consoleplayer = 0;		/* Default - may be overridden later. */
   usemodem=false;

   // Check for auto answer
   if (CheckParm("-answer"))
      {
      usemodem=true;
      answer=true;
      }

   // Check for dial
   if (CheckParm("-dial"))
      {
      usemodem=true;
      answer=false;
      }

   if (CheckParm ("-pause"))
      pause=true;

   // Check for showstats
   if (CheckParm("-stats"))
      {
      showstats=true;
      }

   printf("\n\n ------------\n");

   if (usemodem==true)
      {
      if (answer==false)
         {
         printf(" DIALING MODE\n");
         }
      else
         {
         printf(" ANSWER  MODE\n");
         }
      }
   else
      {
      printf(" SERIAL  MODE\n");
      }
   printf(" ------------\n");

   ReadSetup ();
	GetUart  ();
	InitPort ();

	while ( 0 <= read_byte() );	/* Flush any old stuff from uart buffer */

   if (usemodem==true)
      {
      if (answer==false)
         {
         if (Dial()==FALSE)
            {
            Error("Dialing failed\n");
            }
         }
      else
         {
         if (Answer()==FALSE)
            {
            Error("Answering failed\n");
            }
         }
      }
   if (Connect ())
		{
		reset_counters ();
      talk();
      StartTime();
		LaunchROTT ();
      EndTime();
		}

   if (showstats==true)
      {
      stats();
      }

   ShutDown();

   exit(0);
}
