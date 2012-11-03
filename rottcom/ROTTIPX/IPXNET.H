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

// ipxnet.h

#include "c:\merge\rottnet.h"

typedef struct
{
	char	private[MAXPACKETSIZE];
} rottdata_t;


#include "global.h"

//===========================================================================

#define NUMPACKETS           8 // max outstanding packets before loss
#define NUMPACKETSPERPLAYER  3 // max outstanding packets per player before loss
#define MAXPACKETS           (8*NUMPACKETSPERPLAYER)


// setupdata_t is used as rottdata_t during setup
typedef struct
{
   short client;
	short	playernumber;
	short	command;
   short extra;
   short numplayers;
} setupdata_t;



typedef struct IPXPacketStructure
{
	WORD    PacketCheckSum;         /* high-low */
	WORD    PacketLength;           /* high-low */
	BYTE    PacketTransportControl;
	BYTE    PacketType;

	BYTE    dNetwork[4];            /* high-low */
	BYTE    dNode[6];               /* high-low */
	BYTE    dSocket[2];             /* high-low */

	BYTE    sNetwork[4];            /* high-low */
	BYTE    sNode[6];               /* high-low */
	BYTE    sSocket[2];             /* high-low */
} IPXPacket;


typedef struct
{
	BYTE    network[4];             /* high-low */
	BYTE    node[6];                /* high-low */
} localadr_t;

typedef struct
{
	BYTE    node[6];                /* high-low */
} nodeadr_t;

typedef struct ECBStructure
{
	WORD    Link[2];                /* offset-segment */
	WORD    ESRAddress[2];          /* offset-segment */
	BYTE    InUseFlag;
	BYTE    CompletionCode;
	WORD    ECBSocket;              /* high-low */
	BYTE    IPXWorkspace[4];        /* N/A */
	BYTE    DriverWorkspace[12];    /* N/A */
	BYTE    ImmediateAddress[6];    /* high-low */
	WORD    FragmentCount;          /* low-high */

	WORD    fAddress[2];            /* offset-segment */
	WORD    fSize;                  /* low-high */
	WORD    f2Address[2];            /* offset-segment */
	WORD    f2Size;                  /* low-high */
} ECB;


// time is used by the communication driver to sequence packets returned
// to rott when more than one is waiting

typedef struct
{
	ECB             ecb;
	IPXPacket       ipx;

	long   			time;
	rottdata_t		data;
} packet_t;


extern	nodeadr_t	nodeadr[MAXNETNODES+1];
extern	int			localnodenum;

extern	long   		ipxlocaltime;		// for time stamp in packets
extern	long		remotetime;		// timestamp of last packet gotten

extern	nodeadr_t	remoteadr;

void InitNetwork (void);
void ShutdownNetwork (void);
void SendPacket (int destination);
int  GetPacket (void);
