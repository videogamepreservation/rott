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
#include "sermodem.h"
#include "sersetup.h"
#include "port.h"
#include "..\rottnet.h"
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


#define FRAMECHAR	0x70
#define MAXPACKET (MAXPACKETSIZE)

time_t starttime = 0;
time_t endtime = 0;
time_t playtime = 0;



/* showReadStats() counters. */
unsigned long writeBufferOverruns       = 0;
unsigned long bytesRead                 = 0;
unsigned long packetsRead               = 0;
unsigned long largestReadPacket         = 0;
unsigned long smallestReadPacket        = 0xFFFFFFFFl;
unsigned long readBufferOverruns        = 0;
unsigned long totalReadPacketBytes      = 0;
unsigned long oversizeReadPackets       = 0;
unsigned long largestOversizeReadPacket = 0;
unsigned long overReadPacketLen         = 0;

/* showWriteStats() counters. */
unsigned long bytesWritten               = 0;
unsigned long packetsWrite               = 0;
unsigned long largestWritePacket         = 0;
unsigned long smallestWritePacket        = 0xFFFFFFFFl;
unsigned long totalWritePacketBytes      = 0;
unsigned long oversizeWritePackets       = 0;
unsigned long largestOversizeWritePacket = 0;

/* showUartErrors() counters. */
unsigned long numBreak          = 0;
unsigned long numFramingError   = 0;
unsigned long numParityError    = 0;
unsigned long numOverrunError   = 0;
unsigned long numTxInterrupts   = 0;
unsigned long numRxInterrupts   = 0;

char	packet[MAXPACKET];
char	localbuffer[MAXPACKET*2+2];
int		packetlen;
int		inescape;
int		newpacket;


void showReadStats( void );
void showWriteStats( void );
void showUartErrors( void );



/*
================
=
= ReadPacket
=
================
*/

boolean ReadPacket (void)
{
	int	c;

// if the buffer has overflowed, throw everything out


	if (inque.size > QUESIZE - 4)	// check for buffer overflow
	{
		++readBufferOverruns;           /* Count read overruns */
		inque.tail = inque.head;
		inque.size = 0;
		newpacket = true;
		return false;
	}

	if (newpacket)
	{
		packetlen = 0;
		newpacket = 0;
		overReadPacketLen = 0;
	}

	do
	{
		if ((c = read_byte ()) < 0)
			return false;      // haven't read a complete packet
		if (inescape)
		{
			inescape = false;
			if (c!=FRAMECHAR)
			{
				newpacket = 1;

				++packetsRead;  /* Count packets read */

				if ( packetlen > largestReadPacket ) /* Track largest packet */
					largestReadPacket = packetlen;

				if ( packetlen < smallestReadPacket ) /* Track smallest packet */
					smallestReadPacket = packetlen;

				totalReadPacketBytes += packetlen;    /* Count total packet bytes */

				return true;    // got a good packet
			}
		}
		else if (c==FRAMECHAR)
		{
			inescape = true;
			continue;			// don't know yet if it is a terminator
		}						// or a literal FRAMECHAR

		if (packetlen >= MAXPACKET)
		{
			++overReadPacketLen;			/* Keep track of size of oversize packet */
			oversizeReadPackets++;		/* Count oversize packets */

			if ( overReadPacketLen > largestOversizeReadPacket )
				largestOversizeReadPacket = overReadPacketLen;

			continue;			// oversize packet
		}

		packet[packetlen] = c;
		packetlen++;
	} while (1);

}


/*
=============
=
= WritePacket
=
=============
*/



void WritePacket (char *buffer, int len)
{
	int		b;

	b = 0;
	if (len > MAXPACKET)
	{
		++oversizeWritePackets;         /* Count oversize write packets */
		if ( len > largestOversizeWritePacket )
			++largestOversizeWritePacket;

		return;
	}

	if ( len > largestWritePacket )
		largestWritePacket = len;

	if ( len < smallestWritePacket )
		smallestWritePacket = len;

	totalWritePacketBytes += len;

	++packetsWrite;

	while (len--)
	{
		if (*buffer == FRAMECHAR)
			localbuffer[b++] = FRAMECHAR;	// escape it for literal
		localbuffer[b++] = *buffer++;
	}

	localbuffer[b++] = FRAMECHAR;
	localbuffer[b++] = 0;

	write_buffer (localbuffer, b);
}


/*
=============
=
= NetISR
=
=============
*/
void NetISR ( void )
{
	if (rottcom.command == CMD_SEND)
	{
		WritePacket ((char *)&rottcom.data, rottcom.datalength);
	}
	else if (rottcom.command == CMD_GET)
	{
		if (ReadPacket () && packetlen <= sizeof(rottcom.data) )
		{
			rottcom.remotenode = 1;
			rottcom.datalength = packetlen;
			memcpy (&rottcom.data, &packet, packetlen);
		}
		else
			rottcom.remotenode = -1;
	}
}




/*
=================
=
= Connect
=
= Figures out who is player 0 or 1
=================
*/

int Connect ( void )
{
	struct time		time;
	int				oldsec;
	int		localstage, remotestage;
   int      player;
	char	str[20];

// Check for player specified

   if (CheckParm("-player")!=0)
      {
      player=1;
      }
   else if (CheckParm("-answer"))
      {
      player=1;
      }
   else
      {
      player=0;
      }

//
// wait for a good packet
//
	printf ("Attempting to connect across serial link, press escape to abort.\n");

	rottcom.consoleplayer = player;
	oldsec = -1;
	localstage = remotestage = 0;

	do
	{
		while ( bioskey(1) )
		{
			if ( (bioskey (0) & 0xff) == ESC)
			{
				// Error ("\n\nNetwork game synchronization aborted.");
				printf ("\n\nNetwork game synchronization aborted.\n");
				while (read_byte () != -1)
				;
				return FALSE;
			}
		}

		while (ReadPacket ())
		{
			packet[packetlen] = 0;
			printf ("read: '%s'\n", packet);
			if (packetlen != 7)
			{
				printf ("bad packet len = %d (should be 7)\n", packetlen);
				goto badpacket;
			}
			if (strncmp(packet,"ROTT",4) )
			{
				printf ("error: first 4 char's aren't 'ROTT'\n");
				goto badpacket;
			}
			remotestage = packet[6] - '0';
			localstage = remotestage+1;
			if (packet[4] == '0'+rottcom.consoleplayer)
			{
				rottcom.consoleplayer ^= 1;
				localstage = remotestage = 0;
			}
			oldsec = -1;
		}
badpacket:

		gettime (&time);
		if (time.ti_sec != oldsec)
		{
			oldsec = time.ti_sec;
			sprintf (str,"ROTT%i_%i",rottcom.consoleplayer,localstage);
			WritePacket (str,strlen(str));
			printf ("wrote: '%s'\n",str);
		}

	} while (remotestage < 1);

//
// flush out any extras
//
	while (ReadPacket ())
	;
   delay(20);
	while (ReadPacket ())
	;
	return TRUE;
}

void StartTime( void )
{
   time (&starttime);
}

void EndTime( void )
{
   time (&endtime);
}

void stats (void)
{
	time_t hrs;
	time_t mins;
	time_t secs;

	clrscr ();
	if (starttime != 0)
	{
		printf ("Start Time:    %s", ctime (&starttime));
		playtime = endtime - starttime;
		hrs = playtime / 3600;
		mins = (playtime - (hrs * 3600)) / 60;
		secs = playtime - (hrs * 3600) - (mins * 60);
		printf ("Playing Time:  %d:%02d:%02d\n\n",
			(int) hrs, (int) mins, (int) secs);
	}
	showReadStats();
	showWriteStats();
	showUartErrors();
	printf ("\n");
}



void reset_counters (void)
{
	/* showReadStats() counters. */
	writeBufferOverruns       = 0;
	bytesRead                 = 0;
	packetsRead               = 0;
	largestReadPacket         = 0;
	smallestReadPacket        = 0xFFFFFFFFl;
	readBufferOverruns        = 0;
	totalReadPacketBytes      = 0;
	oversizeReadPackets       = 0;
	largestOversizeReadPacket = 0;
	overReadPacketLen         = 0;

	/* showWriteStats() counters. */
	bytesWritten               = 0;
	packetsWrite               = 0;
	largestWritePacket         = 0;
	smallestWritePacket        = 0xFFFFFFFFl;
	totalWritePacketBytes      = 0;
	oversizeWritePackets       = 0;
	largestOversizeWritePacket = 0;

	/* showUartErrors() counters. */
	numBreak          = 0;
	numFramingError   = 0;
	numParityError    = 0;
	numOverrunError   = 0;
	numTxInterrupts   = 0;
	numRxInterrupts   = 0;
}

void showReadStats()
{
	if ( smallestReadPacket == 0xFFFFFFFFl )
		smallestReadPacket = 0;

	printf ("Read statistics:\n");

	printf ("%9lu Largest packet      %9lu Smallest packet\n",
				largestReadPacket, smallestReadPacket);

	printf ("%9lu Oversize packets    %9lu Largest oversize packet\n",
				oversizeReadPackets, largestOversizeReadPacket);

	printf ("%9lu Total packets       %9lu Buffer overruns\n",
				packetsRead, readBufferOverruns);

	printf ("%9lu Total bytes         %9lu Average bytes/minute\n",
				totalReadPacketBytes,
				starttime == 0 || playtime == 0 ? 0 :
					( ( 60 * totalReadPacketBytes ) / playtime) );

	printf ("%9lu Receive interrupts  %9lu Average bytes/interrupt\n",
		numRxInterrupts,
		numRxInterrupts == 0 ? 0 :
			( totalReadPacketBytes / numRxInterrupts ) );

	printf ("\n");
}



void showWriteStats()
{
	if ( smallestWritePacket == 0xFFFFFFFFl )
		smallestWritePacket = 0;

	printf ("Write statistics:\n");

	printf ("%9lu Largest packet      %9lu Smallest packet\n",
				largestWritePacket, smallestWritePacket);

	printf ("%9lu Oversize packets    %9lu Largest oversize packet\n",
				oversizeWritePackets, largestOversizeWritePacket);

	printf ("%9lu Total packets       %9lu Buffer overruns\n",
				packetsWrite, writeBufferOverruns);

	printf ("%9lu Total bytes         %9lu Average bytes/minute\n",
				totalWritePacketBytes,
				starttime == 0 || playtime == 0 ? 0 :
					( ( 60 * totalReadPacketBytes ) / playtime) );

	printf ("%9lu Transmit interrupts %9lu Average bytes/interrupt\n",
		numTxInterrupts,
		numTxInterrupts == 0 ? 0 :
			( totalReadPacketBytes / numRxInterrupts ) );

	printf ("\n");
}

void showUartErrors()
{
	puts ("UART line status");

	printf ("%9lu Breaks detected     %9lu Framing errors\n",
		numBreak, numFramingError);
	printf ("%9lu Parity errors       %9lu Overrun errors\n",
		numParityError, numOverrunError);
}

