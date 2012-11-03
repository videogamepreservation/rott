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
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <dos.h>
#include <conio.h>
#include "c:\merge\rottnet.h"
#include "global.h"
#if ROTTSER
#include "port.h"
#endif

rottcom_t	rottcom;
int         vectorishooked=0;
boolean     pause;
void interrupt (*oldrottvect) (void);
static char * rottnet_stack;
static unsigned short rottnet_stacksegment;
static unsigned short rottnet_stackpointer;
static unsigned short old_stacksegment;
static unsigned short old_stackpointer;

#define ROTTNET_STACKSIZE    (2048)
#define ROTTNET_SAFETYMARGIN (8)

/*
=============
=
= SetupROTTCOM
=
=============
*/

void SetupROTTCOM ( void )
{
	unsigned char far *vectorptr;
   long vector;
   char * topofstack;


   vector=GetVector();

 	/* Get an interrupt vector if not already set */
	if (vector == -1)
	{
		for (vector = 0x60 ; vector <= 0x66 ; vector++)
		{
			vectorptr = *(unsigned char far * far *)(vector*4);
			if ( !vectorptr || *vectorptr == 0xcf )
				break;
		}
		if (vector == 0x67)
		{
			printf ("Warning: no NULL or iret interrupt vectors were found in the 0x60 to 0x66\n"
					"range.  You can specify a vector with the -vector parameter\n"
					"Press a key to continue...\n");
			getch ();
			printf ("Using default vector 0x66\n");
			vector = 0x66;
		}
	}
	rottcom.intnum = (short)vector;

   // allocate the rottnet stack

   rottnet_stack = malloc(ROTTNET_STACKSIZE);
   if (!rottnet_stack)
      Error("Could not allocate stack");

   // Calculate top of stack

   topofstack = rottnet_stack + ROTTNET_STACKSIZE - ROTTNET_SAFETYMARGIN;

   // Determine stack segment and pointer

   rottnet_stacksegment = FP_SEG( (char huge *)topofstack );
   rottnet_stackpointer = FP_OFF( (char huge *)topofstack );

}


/*
=============
=
= ShutdownROTTCOM
=
=============
*/

void ShutdownROTTCOM ( void )
{
   if (vectorishooked)
		setvect (rottcom.intnum,oldrottvect);
   vectorishooked=0;
   free ( rottnet_stack );
}


/*
=============
=
= GetVector
=
=============
*/

long GetVector (void)
{
   unsigned char far * vector;
   long intnum;


	if (CheckParm ("-vector"))
      {
      intnum = sscanf ("0x%x",_argv[CheckParm("-vector")+1]);
      vector = *(unsigned char far * far *)(intnum*4);
      if (vector != NULL && *vector != 0xcf)
         Error("The specified vector (0x%02x) was already hooked.\n", intnum);
      return intnum;
      }
   else
      return -1;
}

/*
=============
=
= ROTTNET_ISR
=
=============
*/

#define GetStack(a,b) \
   {                  \
   *a = _SS;          \
   *b = _SP;          \
   }
#define SetStack(a,b) \
   {                  \
   _SS=a;             \
   _SP=b;             \
   }

void interrupt ROTTNET_ISR (void)
	{
   //
   // Get current stack
   //

   GetStack( &old_stacksegment, &old_stackpointer );

   //
   // Set the local stack
   //

   SetStack( rottnet_stacksegment, rottnet_stackpointer );

   //
   // call the interrupt service routine
   //

   NetISR();

   //
   // Restore the old stack
   //

   SetStack( old_stacksegment, old_stackpointer );
   }

/*
=============
=
= LaunchROTT
=
=============
*/

void LaunchROTT (void)
{
	char	*newargs[99];
	char	adrstring[10];
	long  	flatadr;
	int argnum = 1;
	int i;

   SetupROTTCOM ();

// prepare for ROTT

	oldrottvect = getvect (rottcom.intnum);
	setvect (rottcom.intnum,ROTTNET_ISR);
	vectorishooked = 1;

// set ticstep

   if (rottcom.gametype == 0) // modem game
      {
      rottcom.ticstep = 2; // skip every other tic
      }
   else // must be a network game
      {
      rottcom.ticstep = 1; // use every tic
      }

// build the argument list for ROTT, adding "-net" and the address of rottcom.

   for (i=1;i<_argc;i++)
	   newargs [argnum++] = _argv[i];

	newargs [argnum++] = "now";

#if ROTTSER
   if (Is8250())
      newargs [argnum++] = "IS8250";
#endif
	newargs [argnum++] = "-net";

	/* Add address of rottcom structure */

	flatadr = (long)_DS*16 + (unsigned)&rottcom;
	sprintf (adrstring,"%lu",flatadr);
	newargs [argnum++] = adrstring;

	newargs [argnum] = NULL;

	newargs [0] = ROTTLAUNCHER;

   if (pause==true)
      {
      printf ("About to launch %s -- Passing these arguments:\n",ROTTLAUNCHER);
      for (i = 0; i < argnum; i++)
         printf ("  arg %d = %s\n", i, newargs [i]);
      printf ("  player = %d\n", rottcom.consoleplayer);

      printf ("\nPress ESC to abort, or any other key to continue...");
      if (getch () == ESC)
         {
         printf ("\n\n");
         return;
         }
      }

	spawnv  (P_WAIT, ROTTLAUNCHER, newargs);
	printf ("\nReturned from ROTT\n\n");

   ShutdownROTTCOM();
}
