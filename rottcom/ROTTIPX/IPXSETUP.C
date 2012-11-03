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

// ipxsetup.c

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <stdarg.h>
#include <bios.h>
#include <time.h>

#include "..\rottnet.h"
#include "ipxnet.h"

#define VERSION "1.3"

int numnetnodes;
unsigned socketid = 0x882a;        // 0x882a is the official ROTT socket
int     myargc;
char **myargv;

boolean server;
boolean standalone;
boolean master;

setupdata_t	nodesetup;
#define MAXNETMESSAGES 25
static int messagesused;
static boolean StringUsed[MAXNETMESSAGES];
static char * NetStrings[MAXNETMESSAGES]=
{
{"\nUm, I'm sure Player %d will join us soon.\n"},
{"\nPlayer %d is starting to tick me off.\n"},
{"\nIt seems Player %d has gone for a Moon Pie.\n"},
{"\nHow long can Player %d take setting the dang thing up?\n"},
{"\nPlayer %d...where are you?\n"},
{"\nSigh.  Player %d is a toadie.\n"},
{"\nGo give Player %d a good swift kick in the...head.\n"},
{"\nIs Player %d running off a removable drive or something?\n"},
{"\nPlayer %d is a popo-head.\n"},
{"\nPlayer %d is attempting to escape off the map.\n"},
{"\nPLAYER %d!!!! GOON!\n"},
{"\nYou know, waiting for Player %d reminds me of a story....\n"},
{"\nTwo Player %d's walk into a bar....\n"},
{"\nHow many Player %d's does it take to change a lightbulb?\n"
   "None, 'cause they don't DO anything.  They just SIT there.\n"},
{"\nANY TIME NOW PLAYER %d!!!\n"},
{"\nI hear Player %d sucks dog's toes.\n"},
{"\nWho votes that Player %d gets left outta this game (y/n)?\n"},
{"\nLunch break's over, Player %d!\n"},
{"\nWe're all waiting, Player %d....  Hello, McFly!!!\n"},
{"\nNumber %d Player, our Number 1 delayer....\n"},
{"\nINSTRUCTIONS: Player %d runs the setup program....\n"},
{"\nOnce Player %d deigns to join us, let's toast 'em.\n"},
{"\nPssst...If you go wake up Player %d, I'll put you in God mode.\n"},
{"\nOkay, when we start, I'm giving Player %d only 5 hit points.\n"},
{"\nIs Player %d in Shrooms Mode or what?\n"}
};

/*
=============
=
= NetISR
=
=============
*/

void NetISR (void)
{
   if (rottcom.command == CMD_SEND)
      {
      ipxlocaltime++;
      SendPacket (rottcom.remotenode);
      }
   else if (rottcom.command == CMD_GET)
      {
      GetPacket ();
      }
}

/*
=================
=
= Shutdown
=
=================
*/

void Shutdown ( void )
{
   ShutdownROTTCOM ();
   ShutdownNetwork();
}

/*
===================
=
= SetupRandomMessages
=
===================
*/
void SetupRandomMessages ( void )
{
   messagesused=0;
   memset(StringUsed,0,sizeof(StringUsed));
   randomize();
}

/*
===================
=
= PrintRandomMessage
=
===================
*/
void PrintRandomMessage (int message, int player)
{
   printf(NetStrings[message],player);
}

/*
===================
=
= GetRandomMessage
=
===================
*/
int GetRandomMessage ( void )
{
   boolean found=false;
   int num;

   if (messagesused==MAXNETMESSAGES)
      SetupRandomMessages();

   while (found==false)
      {
      num = random(MAXNETMESSAGES);
      if (StringUsed[num]==false)
         {
         StringUsed[num]=true;
         found=true;
         messagesused++;
         }
      }
   return num;
}

#define client_NoResponse 0
#define client_Echoed 1
#define client_Done 2

int playernumber;
int clientstate[MAXNETNODES+1];
/*
===================
=
= ResetNodeAddresses
=
= Finds all the nodes for the game and works out player numbers among them
=
===================
*/
void ResetNodeAddresses ( void )
{
   int i;

   // Zero out client state structure

   playernumber=1;

   memset(clientstate,0,sizeof(clientstate));

   for (i=1 ; i<MAXNETNODES ; i++)
      memset (&nodeadr[i],0,sizeof(&nodeadr[i]));

   clientstate[0]=client_Done;

   if (standalone==false)
      {
      clientstate[playernumber]=client_Done;
      rottcom.consoleplayer=playernumber;
      memcpy (&nodeadr[playernumber], &nodeadr[0], //copy in local address
              sizeof(nodeadr[playernumber]) );
      printf ("\nServer is Player %d\n",playernumber);
      playernumber++;
      }
}

/*
===================
=
= LookForNodes
=
= Finds all the nodes for the game and works out player numbers among them
=
===================
*/

#define cmd_FindClient 1
#define cmd_HereIAm 2
#define cmd_YouAre 3
#define cmd_IAm 4
#define cmd_AllDone 5
#define cmd_Info 6

#define MAXWAIT 10


void LookForNodes (void)
{
   int             i;
   struct time     time;
   int             oldsec;
   setupdata_t     *setup;
   boolean         done;
   short           numplayers,extra;
   boolean         showednum;
   short           secondsleft;
   boolean         masterset=false;

   done=false;

   oldsec = -1;
   secondsleft = MAXWAIT;
   setup = (setupdata_t *)&rottcom.data;
   ipxlocaltime = -1;          // in setup time, not game time
   showednum = false;
   SetupRandomMessages ();

   if (server==true)
      {

      printf("SERVER MODE:\n");
      printf("============\n");
      printf("Attempting to find all clients for %i player NETROTT\n",
             numnetnodes);

      nodesetup.client=0;
      rottcom.client=0;
      rottcom.numplayers=numnetnodes;
      ResetNodeAddresses ();
      if (standalone==false)
         {
         masterset=true;
         }
      }
   else
      {
      printf("CLIENT MODE:\n");
      printf("============\n");
      printf("Attempting to find server\n");

      rottcom.numplayers=MAXPLAYERS;
      nodesetup.client=1;
      rottcom.client=1;
      }



//
// build local setup info
//

   nodesetup.playernumber = 0;

   gettime (&time);
   oldsec = time.ti_sec;

   do
      {
      //
      // check for aborting
      //
      while ( bioskey(1) )
         {
         if ( (bioskey (0) & 0xff) == 27)
            Error ("\n\nNetwork game synchronization aborted.");
         }
      if (server==false) // Client code
         {
         //
         // listen to the network
         //
         while (GetPacket ())
            {
            extra      = setup->extra;
            numplayers = setup->numplayers;

            if (remotetime != -1)
               {    // an early game packet, not a setup packet
               if (rottcom.remotenode == -1)
                  Error ("\n\nUnkown game packet: Other ROTT server present");
               }

            if (setup->client==0) // It's a server packet
               {
               switch (setup->command)
                  {
                  case cmd_FindClient:

                     // copy the server's address
                     if (rottcom.remotenode==-1) // only set if it is an unknown pkt
                        {
                        memcpy (&nodeadr[1], &remoteadr,
                               sizeof(nodeadr[1]) );
                        }
                     nodesetup.command=cmd_HereIAm;
                     if (master==true)
                        nodesetup.extra=1;
                     else
                        nodesetup.extra=0;
                     memcpy (&rottcom.data,
                             &nodesetup,sizeof(*setup));
                     rottcom.datalength = sizeof(*setup);
                     SendPacket (1);     // send to server
                     printf (".");
                     break;
                  case cmd_Info:
                     if (showednum==false)
                        {
                        printf("\nServer is looking for %d players\n",numplayers);
                        showednum = true;
                        }
                     if ((extra>>8)!=0)
                        {
                        if ((extra>>8)==100)
                           printf("\nServer found player %d\n",extra&0xff);
                        else
                           PrintRandomMessage((extra>>8)-1,extra&0xff);
                        }
                     break;
                  case cmd_YouAre:
                     rottcom.consoleplayer=setup->playernumber;
                     nodesetup.command=cmd_IAm;
                     nodesetup.playernumber=setup->playernumber;
                     memcpy (&rottcom.data,
                             &nodesetup,sizeof(*setup));
                     rottcom.datalength = sizeof(*setup);
                     SendPacket (1);     // send to server
                     printf (".");
                     printf ("\nI am player %d\n",setup->playernumber);
                     break;
                  case cmd_AllDone:
                     rottcom.numplayers=setup->playernumber;
                     done=true;
                     printf ("!");
                     break;
                  }
               }
            }
         }
      else // It's the server
         {
         //
         // listen to the network
         //
         while (GetPacket ())
            {
            extra      = setup->extra;
            if (remotetime != -1)
               {    // an early game packet, not a setup packet
               if (rottcom.remotenode == -1)
                  Error ("\n\nUnkown game packet: Other clients still in game.");
               }
            if (setup->client==1) // It's a client packet
               {
               switch (setup->command)
                  {
                  case cmd_HereIAm:
                     {
                     int pnum;

                     if ((masterset==false) && (extra==1)) // some client is using the 'master' param
                        {
                        ResetNodeAddresses ();
                        masterset=true;
                        rottcom.remotenode=-1;
                        }

                     pnum=playernumber;
                     if (rottcom.remotenode==-1) // only set if it is an unknown pkt
                        {
                        // copy the client's address
                        printf ("\nFound Player %d\n",pnum);
                        memcpy (&nodeadr[playernumber], &remoteadr,
                               sizeof(nodeadr[playernumber]) );
                        nodesetup.extra=(short)(100<<8);
                        nodesetup.extra |= playernumber;
                        nodesetup.command=cmd_Info;
                        nodesetup.numplayers=numnetnodes;
                        rottcom.datalength = sizeof(*setup);
                        memcpy (&rottcom.data, &nodesetup,sizeof(*setup));
                        SendPacket (MAXNETNODES);     // send to all
                        }
                     else
                        pnum=rottcom.remotenode;
                     if (clientstate[pnum]>client_Echoed)
                        continue;
                     nodesetup.command=cmd_YouAre;
                     nodesetup.playernumber=pnum;
                     memcpy (&rottcom.data,
                             &nodesetup,sizeof(*setup));
                     SendPacket (pnum);   // send back to client
                     clientstate[pnum]=client_Echoed;
                     playernumber++;
                     }
                     break;
                  case cmd_IAm:
                     if (rottcom.remotenode==-1) // Shouldn't happen
                        continue;
//                      Error("\n\nReceived Identity packet before identification\n");
                     if (rottcom.remotenode!=setup->playernumber)
                        Error("\n\nReceived Incorrect player number\n");
                     printf ("Finished Player %d\n",rottcom.remotenode);
                     clientstate[rottcom.remotenode]=client_Done;
                     break;
                  }
               }
            else
               {
               if (rottcom.remotenode!=0)
                  Error("\n\nMultiple ROTT Servers!\n");
               switch (setup->command)
                  {
                  case cmd_Info:
                     if (((extra>>8)!=0) && (standalone==false))
                        {
                        if ((extra>>8)==100)
                           printf("\nServer found player %d\n",extra&0xff);
                        else
                           PrintRandomMessage((extra>>8)-1,extra&0xff);
                        }
                     break;
                  }
               }
            }

            // Check to see if we are indeed done

            done=true; // set it to true
            if (playernumber > numnetnodes+1)
               Error("\n\nFound too many players=%d\n",playernumber);
            if (playernumber <= numnetnodes)
               done=false;
            else
               {
               for (i=0;i<playernumber;i++)
                  if (clientstate[i]!=client_Done)
                     done=false;
               }
            gettime (&time);

            if (time.ti_sec == oldsec)
               continue;

            secondsleft--;
            if (secondsleft==0)
               {
               secondsleft=MAXWAIT;
               nodesetup.extra=(GetRandomMessage())+1;
               nodesetup.extra<<=8;
               nodesetup.extra|=playernumber;
               nodesetup.command=cmd_Info;
               nodesetup.numplayers=numnetnodes;
               rottcom.datalength = sizeof(*setup);
               memcpy (&rottcom.data, &nodesetup,sizeof(*setup));

               SendPacket (MAXNETNODES);     // send to all
               }

            oldsec = time.ti_sec;
            printf (".");

            // Make a general inquiry for any potential ROTT players
            nodesetup.extra=0;
            nodesetup.command=cmd_FindClient;
            nodesetup.numplayers=numnetnodes;
            rottcom.datalength = sizeof(*setup);
            memcpy (&rottcom.data, &nodesetup,sizeof(*setup));

            SendPacket (MAXNETNODES);     // send to all
         }

      } while (done==false);

// Done with finding players send out startup info

   if (server==true)
      {
      int otime;

      gettime (&time);
      oldsec = time.ti_sec+1;
      if (oldsec>59)
         oldsec-=60;
      otime = time.ti_sec-1;
      while (oldsec!=time.ti_sec)
         {
         gettime (&time);

         if (time.ti_sec == otime)
            continue;
         otime = time.ti_sec;
         nodesetup.command=cmd_AllDone;
         nodesetup.playernumber=playernumber-1;
         rottcom.datalength = sizeof(*setup);
         memcpy (&rottcom.data, &nodesetup,sizeof(*setup));

         SendPacket (MAXNETNODES);     // send to all
         }
      }

   if (server==true)
      {
      if (standalone == false)
         printf ("Server is player %i of %i\n", rottcom.consoleplayer, rottcom.numplayers);
      else
         printf ("Server is standalone\n");
      }
   else
      printf ("\nConsole is player %i of %i\n", rottcom.consoleplayer, rottcom.numplayers);
   while (GetPacket ()) {}
}

/*
=============
=
= main
=
=============
*/

void main (void)
{
	int	i;

//
// determine game parameters
//
	numnetnodes = 2;
   rottcom.gametype=NETWORK_GAME;
   rottcom.remoteridicule = 0;


	printf("\n\n                     -----------------------------------\n");
   printf("                       ROTT NETWORK DEVICE DRIVER V%s \n",VERSION);
   printf("                     -----------------------------------\n");

	i = CheckParm ("-nodes");
	if (i && i < _argc-1)
      {
		numnetnodes = atoi(_argv[i+1]);
      }

	i = CheckParm ("-socket");
	if (i && i < _argc-1)
      {
		socketid = atoi(_argv[i+1]);
      }

   server=false;
   standalone=false;
   pause=false;
   master=false;

	if (CheckParm ("-server"))
      server=true;

	if (CheckParm ("-standalone"))
      standalone=true;

	if (CheckParm ("-pause"))
      pause=true;

	if (CheckParm ("-master"))
      master=true;

	if (CheckParm ("-remoteridicule"))
      rottcom.remoteridicule = 1;

	InitNetwork ();

	LookForNodes ();

	ipxlocaltime = 0;			// no longer in setup

	LaunchROTT ();

	Shutdown ();

   exit(0);
}


