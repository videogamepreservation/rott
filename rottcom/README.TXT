ZIP contents


ROTTIPX      <DIR>         IPX source BC 3.1 format
ROTTSER      <DIR>         SERIAL source BC 3.1 format
ROTTNET  C           4,861 ROTT<->DRIVER interface code
ROTTNET  H           1,955 "
RT_NET   H           7,008 ROTT packet headers with size info
README   TXT         3,366 This file


ROTT Network Specification:

ROTT does all of it's multi-player communication through a user interrupt
in the range of 0x60 - 0x66.  This is all very similar to DOOM.


Data is passed back in forth between ROTT and the real mode driver by
means of the ROTTNET structure defined in "ROTTNET.H".  "ROTTNET.H" should
not be changed, since it is compiled into ROTT itself.  Here is a
description of what each item in the ROTTNET structure does:

typedef struct
   {
   //
   // The interrupt on which ROTT and the caller are to communicate
   //
   short   intnum;  // ROTT executes an int to send commands

   // communication between ROTT and the driver

   //
   // what type of command we are sending to the real mode driver,
   // a get packet or send packet command
   //

   short   command;                // CMD_SEND or CMD_GET

   //
   // the destination node when we send packets, and the source node when
   // we receive packets.  In network games, the following convention is
   // used for player numbers:
   //
   // For Clients:
   //
   // address 0: their local address
   // address 1: the address of the server
   //
   // For the Packet Server:
   // address 0: local address
   // address 1: address of player 1, if standalone
   //            local address if client on top of server
   //            but still the address of player 1
   // address 2: address of player 2
   // address 3: address of player 3
   // address 4: address of player 4
   //  .
   //  .
   //  .
   //
   // In MODEM games the destination and source are pretty unused since
   // only two player are capable of playing.
   //
   // upon getting a packet, if remotenode = -1 there is no packet ready
   //
   short   remotenode;  // dest for send, set by get (-1 = no packet)

   //
   // length the packet to send
   // length of the packet received
   //
   short   datalength;  // bytes in rottdata to be sent / bytes read

   // info specific to this node

   //
   // The player number of this node
   // In MODEM games it is either 0 or 1
   //
   // IN NETWORK games
   //
   // player 1 = 1
   // player 2 = 2
   // player 3 = 3
   // etc.
   // passing in a consoleplayer value of 0 signifies standalone server
   //
   short   consoleplayer;  // 0-(numplayers-1) = player number

   //
   // numplayers in the game
   //
   short   numplayers;     // 1-11

   //
   // whether the current computer is a server or a client
   //
   short   client;         // 0 = server 1 = client

   //
   // whether we are playing a modem or network game
   //
   short   gametype;       // 0 = modem  1 = network

   //
   // space between packet times
   //
   // 1 = 35   FPS control update speed
   // 2 = 17.5 FPS control update speed
   // 3 = 11.7 FPS control update speed
   // 4 = 8.75 FPS control update speed
   //
   short   ticstep;        // 1 for every tic 2 for every other tic ...

   //
   // whether this station is capable of sending live remote ridicule
   // ticstep is set to 1 if this is enabled.
   // bandwith of comm device must be capable of 256*35 = 8960 bytes per
   // second.

   short   remoteridicule; // 0 = remote ridicule is off 1= rr is on

   // packet data to be sent
   char    data[MAXPACKETSIZE];
   } rottcom_t;


If you have any questions give me a call or drop me a note

Mark Dochtermann
(214) 271 - 1365 ext. 210
paradigm@metronet.com
