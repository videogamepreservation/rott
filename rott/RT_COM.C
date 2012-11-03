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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <process.h>
#include <stdarg.h>
#include <bios.h>
#include <ctype.h>
#include "rt_def.h"
#include "_rt_com.h"
#include "rt_com.h"
#include "rt_util.h"
#include "rt_in.h"
#include "rt_crc.h"
#include "rt_playr.h"
#include "isr.h"
#include "rt_msg.h"
#include "rottnet.h"
#include "rt_main.h"
#include "rt_net.h"
#include "rt_draw.h"
//#include "rt_ser.h"
//MED
#include "memcheck.h"

// GLOBAL VARIABLES

// Same as in real mode
rottcom_t   *	rottcom;
int badpacket;
int consoleplayer;
byte ROTTpacket[MAXCOMBUFFERSIZE];
int controlsynctime;

// LOCAL VARIABLES
static union  REGS   comregs;
static int    ComStarted=false;
static int    transittimes[MAXPLAYERS];


/*
===============
=
= InitROTTNET
=
===============
*/

void InitROTTNET (void)
{
	int netarg;
	long netaddress;

	if (ComStarted==true)
		return;
	ComStarted=true;

	netarg=CheckParm ("net");
	netarg++;

	netaddress=atol(_argv[netarg]);
	rottcom=(rottcom_t *)netaddress;
   remoteridicule = false;
   remoteridicule = rottcom->remoteridicule;
   if (rottcom->ticstep != 1)
      remoteridicule = false;
   if (remoteridicule == true)
      {
      if (!quiet)
         printf("ROTTNET: LIVE Remote Ridicule Enabled\n");
      }

   if (!quiet)
      {
      printf("ROTTNET: Communicating on vector %ld\n",rottcom->intnum);
      printf("ROTTNET: consoleplayer=%ld\n",rottcom->consoleplayer);
      }
}

/*
================
=
= ReadPacket
=
================
*/

boolean ReadPacket (void)
{
   word   crc;
   word   sentcrc;

   // Set command (Get Packet)
	rottcom->command=CMD_GET;

   badpacket = 0;

   // Check to see if a packet is ready

	int386(rottcom->intnum,&comregs,&comregs);

   // Is it ready?

   if (rottcom->remotenode!=-1)
      {
      // calculate crc on packet
      crc=CalculateCRC (&rottcom->data[0], rottcom->datalength-sizeof(word));

      // get crc inside packet
      sentcrc=*((word *)(&rottcom->data[rottcom->datalength-sizeof(word)]));

      // are the crcs the same?
      if (crc!=sentcrc)
         {
         badpacket=1;
         SoftError("BADPKT at %ld\n",ticcount);
         }
      if (networkgame==false)
         {
         rottcom->remotenode=server;
         }
      else
         {
         if ((IsServer==true) && (rottcom->remotenode>0))
            rottcom->remotenode--;
         }
      memcpy(&ROTTpacket[0], &rottcom->data[0], rottcom->datalength);

//      SoftError( "ReadPacket: time=%ld size=%ld src=%ld type=%d\n",ticcount, rottcom->datalength,rottcom->remotenode,rottcom->data[0]);

#if 0
      rottcom->command=CMD_OUTQUEBUFFERSIZE;
      int386(rottcom->intnum,&comregs,&comregs);
      SoftError( "outque size=%ld\n",*((short *)&(rottcom->data[0])));
      rottcom->command=CMD_INQUEBUFFERSIZE;
      int386(rottcom->intnum,&comregs,&comregs);
      SoftError( "inque size=%ld\n",*((short *)&(rottcom->data[0])));
#endif
      return true;
      }
   else // Not ready yet....
      return false;
}


/*
=============
=
= WritePacket
=
=============
*/

void WritePacket (void * buffer, int len, int destination)
{
   word      crc;

   // set send command
	rottcom->command=CMD_SEND;

   // set destination
   rottcom->remotenode=destination;

   if (len>(MAXCOMBUFFERSIZE-sizeof(word)))
      {
      Error("WritePacket: Overflowed buffer\n");
      }

   // copy local buffer into realmode buffer
   memcpy((byte *)&(rottcom->data[0]),(byte *)buffer,len);

   // calculate CRC
   crc=CalculateCRC (buffer, len);

   // put CRC into realmode buffer packet
   *((word *)&rottcom->data[len])=crc;

   // set size of realmode packet including crc
   rottcom->datalength=len+sizeof(word);

   if (*((byte *)buffer)==0)
       Error("Packet type = 0\n");

   if (networkgame==true)
      {
      if (IsServer==true)
         rottcom->remotenode++; // server fix-up
      }

//   SoftError( "WritePacket: time=%ld size=%ld src=%ld type=%d\n",ticcount,rottcom->datalength,rottcom->remotenode,rottcom->data[0]);
   // Send It !
	int386(rottcom->intnum,&comregs,&comregs);

#if 0
   rottcom->command=CMD_OUTQUEBUFFERSIZE;
   int386(rottcom->intnum,&comregs,&comregs);
   SoftError( "outque size=%ld\n",*((short *)&(rottcom->data[0])));
   rottcom->command=CMD_INQUEBUFFERSIZE;
   int386(rottcom->intnum,&comregs,&comregs);
   SoftError( "inque size=%ld\n",*((short *)&(rottcom->data[0])));
#endif
}




/*
=============
=
= ValidSyncPacket
=
=============
*/
boolean ValidSyncPacket ( synctype * sync )
{
   if (ReadPacket() && (badpacket==0))
      {
      if (((syncpackettype *)&(ROTTpacket[0]))->type==COM_SYNC)
         {
         memcpy(&(sync->pkt),&(ROTTpacket[0]),sizeof(sync->pkt));
         return true;
         }
      }
   return false;
}

/*
=============
=
= SendSyncPacket
=
=============
*/
void SendSyncPacket ( synctype * sync, int dest)
{
   sync->pkt.type=COM_SYNC;
   sync->sendtime=ticcount;
   WritePacket( &(sync->pkt.type) , sizeof(syncpackettype) , dest );
}


/*
=============
=
= SlavePhaseHandler
=
=============
*/

boolean SlavePhaseHandler( synctype * sync )
{
   boolean done;

   done=false;

   switch (sync->pkt.phase)
      {
      case SYNC_PHASE1:
         break;
      case SYNC_PHASE2:
         ISR_SetTime(sync->pkt.clocktime);
         break;
      case SYNC_PHASE3:
         sync->pkt.clocktime=ticcount;
         break;
      case SYNC_PHASE4:
         ISR_SetTime(ticcount-sync->pkt.delta);
         sync->pkt.clocktime=ticcount;
         break;
      case SYNC_PHASE5:
         ISR_SetTime(ticcount-sync->pkt.delta);
         sync->sendtime=sync->pkt.clocktime;
         done=true;
         break;
      }
   return done;
}



/*
=============
=
= MasterPhaseHandler
=
=============
*/

boolean MasterPhaseHandler( synctype * sync )
{
   boolean done;

   done=false;

   switch (sync->pkt.phase)
      {
      case SYNC_PHASE1:
         sync->pkt.phase=SYNC_PHASE2;
         sync->pkt.clocktime=ticcount+(sync->deltatime>>1);
         break;
      case SYNC_PHASE2:
         sync->pkt.phase=SYNC_PHASE3;
         break;
      case SYNC_PHASE3:
         sync->pkt.delta=sync->pkt.clocktime-ticcount+(sync->deltatime>>1);
         sync->pkt.phase=SYNC_PHASE4;
         break;
      case SYNC_PHASE4:
         sync->pkt.phase=SYNC_PHASE5;
         sync->pkt.delta=sync->pkt.clocktime-ticcount+(sync->deltatime>>1);
         sync->sendtime=ticcount+SYNCTIME;
         sync->pkt.clocktime=sync->sendtime;
         done=true;
         break;
      }
   return done;
}


/*
=============
=
= SetTime
=
=============
*/

void SetTime ( void )
{
   int i;
   syncpackettype * syncpacket;
   boolean done=false;

   syncpacket=(syncpackettype *)SafeMalloc(sizeof(syncpackettype));


   // Sync clocks

   if (networkgame==true)
      {
      if (IsServer==true)
         {
         for (i=0;i<numplayers;i++)
            {
            if (PlayerInGame(i)==false)
               continue;
            if (standalone==true)
               SyncTime(i);
            else if (i!=consoleplayer)
               SyncTime(i);
            if (standalone==true)
               printf("SetTime: player#%ld\n",i);
            }
         }
      else
         {
         SyncTime(0);
         }
      }
   else // Modem 2-player game
      {
      if (consoleplayer==0)
         SyncTime(server);
      else
         SyncTime(server);
      }

   if ( ( (networkgame==true) && (IsServer==true) ) ||
        ( (networkgame==false) && (consoleplayer==0) )
      ) // Master/Server
      {
      int nump;
      int time;

      syncpacket->type=COM_START;
      syncpacket->clocktime=ticcount;
      controlsynctime=syncpacket->clocktime;
      if (networkgame==true)
         nump=numplayers;
      else
         nump=1;

      time = ticcount;

      for (i=0;i<nump;i++)
         {
         WritePacket( &(syncpacket->type) , sizeof(syncpackettype) , i );
         }

      while (ticcount<time+(VBLCOUNTER/4)) ;

      for (i=0;i<nump;i++)
         {
         WritePacket( &(syncpacket->type) , sizeof(syncpackettype) , i );
         }

      if (standalone==true)
         printf("SetTime: Start packets sent\n");
      }
   else // Slave/Client
      {
      while (done==false)
         {
         AbortCheck("SetTime aborted as client");

         if (ReadPacket() && (badpacket==0))
            {
            memcpy(syncpacket,&(ROTTpacket[0]),sizeof(syncpackettype));
            if (syncpacket->type==COM_START)
               {
               controlsynctime=syncpacket->clocktime;
               done=true;
               }
            }
         }
      }
   if (standalone==false)
      {
      AddMessage("All players synched.",MSG_SYSTEM);
      ThreeDRefresh();
      }
   SafeFree(syncpacket);

//
// flush out any extras
//
   while (ticcount<controlsynctime+VBLCOUNTER)
      {
      ReadPacket ();
      }
}

/*
=============
=
= InitialMasterSync
=
=============
*/
void InitialMasterSync ( synctype * sync, int client )
{
   boolean done=false;
   int i;

   if (networkgame==true)
      {
      for (i=0;i<numplayers;i++)
         {
         if (i<=client)
            continue;
         sync->pkt.type=COM_SYNC;
         sync->pkt.phase=SYNC_MEMO;
         sync->pkt.clocktime=client;
         SendSyncPacket(sync,i);
         }
      }

   // Initialize send time so as soon as we enter the loop, we send

   sync->sendtime=ticcount-SYNCTIME;

   while (done==false)
      {
      sync->pkt.phase=SYNC_PHASE0;

      AbortCheck("Initial sync aborted as master");
	   if ((sync->sendtime+SYNCTIME) <= ticcount)
         SendSyncPacket(sync,client);
      if (ValidSyncPacket(sync)==true)
         {
         if (sync->pkt.phase==SYNC_PHASE0)
            {
            int time=ticcount;

            while (time+SYNCTIME>ticcount)
               {
               ReadPacket();
               }
            time=ticcount;
            while (time+SYNCTIME>ticcount) {}
            done=true;
            }
         }
      }
}

/*
=============
=
= InitialSlaveSync
=
=============
*/
void InitialSlaveSync ( synctype * sync )
{
   boolean done=false;

   while (done==false)
      {
      AbortCheck("Initial sync aborted as slave");
      if (ValidSyncPacket(sync)==true)
         {
         if (sync->pkt.phase==SYNC_MEMO)
            {
            char str[50]="Server is synchronizing player ";
            char str2[10];

            strcat(str,itoa(sync->pkt.clocktime+1,str2,10));
            AddMessage(str,MSG_SYSTEM);
            ThreeDRefresh();
            }
         if (sync->pkt.phase==SYNC_PHASE0)
            {
            int time=ticcount;

            SendSyncPacket(sync,server);
            while (time+SYNCTIME>ticcount)
               {
               ReadPacket();
               }
            done=true;
            }
         }
      }
   AddMessage("Server is synchronizing your system",MSG_SYSTEM);
   ThreeDRefresh();
}


/*
=============
=
= SyncTime
=
=============
*/

void SyncTime( int client )
{
   int dtime[NUMSYNCPHASES];
   boolean done;
   int i;
   synctype * sync;

   sync=(synctype *)SafeMalloc(sizeof(synctype));

   if ( ((networkgame==true) && (IsServer==true)) ||
         ((networkgame==false) && (consoleplayer==0)) )
      {
      // Master

      InitialMasterSync ( sync, client );

      done=false;

      // Initial setup for Master
      // Initialize send time so as soon as we enter the loop, we send

      sync->pkt.phase=SYNC_PHASE1;
      sync->sendtime=ticcount-SYNCTIME;

      while (done==false)
         {
         // Master

         AbortCheck("SyncTime aborted as master");

		   if ((sync->sendtime+SYNCTIME) <= ticcount)
            SendSyncPacket(sync,client);

         while (ValidSyncPacket(sync)==true)
            {

            // find average delta

            sync->deltatime=0;

            // calculate last delta

            dtime[sync->pkt.phase]=ticcount-sync->sendtime;

            for (i=0;i<=sync->pkt.phase;i++)
               sync->deltatime+=dtime[i];
            if (i!=0)
               sync->deltatime/=i;
            else
               Error("SyncTime: this should not happen\n");

            done = MasterPhaseHandler( sync );

            SendSyncPacket(sync,client);

            }
         }
      }
   else
      {
      // Slave

      InitialSlaveSync ( sync );

      done=false;

      while (done==false)
         {
         // Slave

         AbortCheck("SyncTime aborted as slave");

         while (ValidSyncPacket(sync)==true)
            {
            done = SlavePhaseHandler( sync );

            if (done==false)
               SendSyncPacket(sync,server);

            }
         }
      }

   while (sync->sendtime > ticcount)
      {
      while (ReadPacket()) {}
      }
   while ((sync->sendtime+SYNCTIME) > ticcount)
      {
      }

   if ( ((networkgame==true) && (IsServer==true)) ||
         ((networkgame==false) && (consoleplayer==0)) )
      SetTransitTime( client, (sync->deltatime>>1));

   SafeFree(sync);
}

/*
=============
=
= SetTransitTime
=
=============
*/

void SetTransitTime( int client, int time )
{
   transittimes[client]=time;
}

/*
=============
=
= GetTransitTime
=
=============
*/

int GetTransitTime( int client )
{
   return transittimes[client];
}

