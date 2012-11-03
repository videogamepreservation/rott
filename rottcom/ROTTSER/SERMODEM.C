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

#include "global.h"
#include <time.h>
#include "serial.h"
#include <stdio.h>
#include <string.h>
#include <bios.h>
#include "port.h"
#include "sermodem.h"
#include "sersetup.h"


char initstring[100];
char dialstring[60];
boolean pulse=false;
char hangupstring[60];


/*
=================
=
= hangup_modem
=
=================
*/

void hangup_modem ( void )
{
	printf ("Dropping DTR\n");
	OUTPUT( uart + MODEM_CONTROL_REGISTER
		, INPUT( uart + MODEM_CONTROL_REGISTER ) & ~MCR_DTR );
	delay (1250);
	OUTPUT( uart + MODEM_CONTROL_REGISTER
		, INPUT( uart + MODEM_CONTROL_REGISTER ) | MCR_DTR );
	ModemCommand("+++");
	delay (1250);
	if (hangupstring [0] != EOS)
		ModemCommand(hangupstring);
	else
	{
		printf ("Warning: Null Hangup string. Using default.\n");
		ModemCommand("ATH0");
	}
	delay (1250);
	while (read_byte () != -1)
	;
}


/*
==============
=
= ModemCommand
=
==============
*/

void ModemCommand (char *str)
{
	int i;
	char *ptr;

	printf ("Modem command : ");
	ptr = str;
	for (i = 0; i < strlen (str); i++)
	{
      printf ("%c",*ptr);
		write_buffer (ptr++, 1);
		delay (100);
	}
   printf("\n");
	write_buffer ("\r",1);
}


/*
==============
=
= ModemResponse
=
= Waits for OK, RING, CONNECT, etc
==============
*/


int ModemResponse (char *resp)
{
	int		c;
	int		respptr;
	char	response[80];

	do
	{
		printf ("Modem response: ");
		respptr=0;
		do
		{
			while ( bioskey(1) )
			{
				if ( (bioskey (0) & 0xff) == ESC)
				{
					printf ("\nModem response aborted.\n");
					return FALSE;
				}
			}
			c = read_byte ();
			if (c==-1)
				continue;
			if (c=='\n' || respptr == 79)
			{
				response[respptr] = 0;
				printf ("%s\n",response);
				break;
			}
			if (c>=' ')
			{
				response[respptr] = c;
				respptr++;
			}
		} while (1);

	} while (strncmp(response,resp,strlen(resp)));
	return TRUE;
}


/*
=============
=
= InitModem
=
=============
*/

int InitModem ( void )
{
	if (initstring [0] != EOS)
	{
		ModemCommand (initstring);
		if (! ModemResponse ("OK"))
			return FALSE;
	}
   return TRUE;
}


/*
=============
=
= Dial
=
=============
*/

int Dial ( void )
{
	char	cmd[80];

	usemodem = true;
	InitModem ();

	printf ("Dialing %s\n", dialstring);
	if (pulse==true)
		sprintf (cmd,"ATDP%s", dialstring);
	else
		sprintf (cmd,"ATDT%s", dialstring);

	ModemCommand(cmd);

	return ModemResponse ("CONNECT");

}

/*
=============
=
= Answer
=
=============
*/

int Answer (void)
{
	usemodem = true;
	InitModem ( );
	printf ("Waiting for ring...\n");

	if (! ModemResponse ("RING"))
		return FALSE;
	ModemCommand ("ATA");
	return ModemResponse ("CONNECT");
}
