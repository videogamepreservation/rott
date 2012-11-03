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

//***************************************************************************
//
//   RT_NET.H - Network stuff
//
//***************************************************************************

#ifndef _rt_net_public
#define _rt_net_public

#include "rottnet.h"

#define MAXCMDS  256

#define COM_DELTA   1
#define COM_REQUEST 2
#define COM_FIXUP   3
#define COM_TEXT    4
#define COM_PAUSE   5
#define COM_QUIT    6
#define COM_SYNC    7
#define COM_REMRID  8
#define COM_RESPAWN 10
#define COM_UNPAUSE 11
#define COM_SERVER  12
#define COM_START   13
#define COM_GAMEDESC 15
#define COM_GAMEPLAY 16
#define COM_GAMEMASTER 17
#define COM_GAMEACK 18
#define COM_ENDGAME  19
#define COM_SYNCTIME 20
#if (SYNCCHECK==1)
#define COM_SYNCCHECK 21
#endif
#define COM_SOUNDANDDELTA  22
#define COM_EXIT    23
#define COM_GAMEEND 24
#define COM_DELTANULL 25

#define COM_MAXTEXTSTRINGLENGTH 33

// Sound defines for Remote ridicule

#define COM_SOUND_START_TRANSMISSION  (0xff)
#define COM_SOUND_END_TRANSMISSION    (0xfe)
#define COM_SOUND_NORMAL_TRANSMISSION (0xfd)
#define COM_SOUND_BUFFERSIZE 256

// Demo Delta Structure
typedef struct DemoType {
  int   time;
  short momx;
  short momy;
  word  dangle;
  word  buttons;
} DemoType;

// Demo Header Structure
typedef struct DemoHeaderType {
  gametype demostate;
} DemoHeaderType;

// Movement Queue Structure
typedef struct MoveType {
  byte  type;
  int   time;
  short momx;
  short momy;
  word  dangle;
  word  buttons;
  char  Sounddata[];
} MoveType;

typedef struct NullMoveType {
  byte  type;
  int   time;
} NullMoveType;

typedef struct {

  void * Commands[MAXCMDS];

} CommandType;

typedef struct {

  byte CommandStates[MAXCMDS];

} CommandStatusType;

typedef MoveType COM_SoundAndDeltaType;

typedef struct {
  byte  type;
  byte  data[COM_SOUND_BUFFERSIZE];
} COM_SoundType;

typedef struct {
  byte  type;
  int   synctime;
} COM_SyncType;

typedef struct {
  byte  type;
  int   time;
  int   synctime;
  int   x;
  int   y;
  int   z;
  word  angle;
  word  randomindex;
} COM_CheckSyncType;

typedef struct {
  byte  type;
  int   time;
  byte  numpackets;
  byte  data;
} COM_ServerHeaderType;

typedef struct {
  byte  type;
  int   time;
  byte   numpackets;
} COM_RequestType;

typedef struct {
  byte  type;
  int   time;
  byte  towho;
  char  string[COM_MAXTEXTSTRINGLENGTH];
} COM_TextType;

#define MSG_DIRECTED_TO_ALL  255
#define MSG_DIRECTED_TO_TEAM 254

typedef struct {
  byte  type;
  int   time;
  byte  player;
  byte  num;
  byte  towho;
} COM_RemoteRidiculeType;

typedef struct {
  byte  type;
  int   time;
  byte  numpackets;
  byte  data;
} COM_FixupType;

typedef struct {
  byte  type;
  int   time;
} COM_QuitType;

typedef struct {
  byte  type;
  int   time;
} COM_ExitType;

typedef struct {
  byte  type;
  int   time;
} COM_GameEndType;

typedef struct {
  byte  character;    // which character
  byte  uniformcolor; // which color
  char  codename[MAXCODENAMELENGTH];  // codename
} COM_PlayerDescriptionType;

typedef struct {
  byte  type;
  byte  player;    // which player
  byte  violence;
  byte  Product;
  unsigned Version;

  COM_PlayerDescriptionType playerdescription;
} COM_GamePlayerType;

typedef struct {
  byte  type;
  byte  level;
  word  mapcrc;
  byte  violence;
  byte  Product;
  byte  mode;
  unsigned Version;
  boolean teamplay;
  specials SpecialsTimes;
  battle_type options;
  char  battlefilename[20];
  int   randomseed;
  boolean ludicrousgibs;
  COM_PlayerDescriptionType players[MAXPLAYERS];
} COM_GameMasterType;

typedef struct {
  byte  type;
  byte  player; // which player
} COM_GameAckType;

typedef struct {
  byte  type;
  int   time;
} COM_EndGameType;

typedef struct {
  byte  type;
  int   time;
} COM_RespawnType;

typedef struct {
  byte  type;
  int   time;
} COM_PauseType;

typedef struct {
  byte  type;
  int   time;
} COM_UnPauseType;

#endif


//
// RT_NET.C packet size stuff
//

//****************************************************************************
//
// GetPacketSize ()
//
//****************************************************************************

int GetPacketSize (void * pkt)
{
   int size;

   switch (((MoveType *)pkt)->type)
      {
      case COM_DELTA:
         size=sizeof(MoveType);
         break;
      case COM_DELTANULL:
         size=sizeof(NullMoveType);
         break;
      case COM_REQUEST:
         size=sizeof(COM_RequestType);
         break;
      case COM_FIXUP:
         size=sizeof(COM_FixupType);
         break;
      case COM_TEXT:
         size=sizeof(COM_TextType);
         break;
      case COM_PAUSE:
         size=sizeof(COM_PauseType);
         break;
      case COM_QUIT:
         size=sizeof(COM_QuitType);
         break;
      case COM_EXIT:
         size=sizeof(COM_ExitType);
         break;
      case COM_REMRID:
         size=sizeof(COM_RemoteRidiculeType);
         break;
      case COM_RESPAWN:
         size=sizeof(COM_RespawnType);
         break;
      case COM_UNPAUSE:
         size=sizeof(COM_UnPauseType);
         break;
      case COM_SERVER:
         size=sizeof(COM_ServerHeaderType);
         size-=sizeof(byte);
         break;
      case COM_GAMEDESC:
         size=sizeof(COM_GamePlayerType);
         break;
      case COM_GAMEEND:
         size=sizeof(COM_GameEndType);
         break;
      case COM_GAMEPLAY:
         size=DUMMYPACKETSIZE;
         break;
      case COM_GAMEACK:
         size=sizeof(COM_GameAckType);
         break;
      case COM_GAMEMASTER:
         size=sizeof(COM_GameMasterType);
         break;
      case COM_ENDGAME:
         size=sizeof(COM_EndGameType);
         break;
      case COM_SYNCTIME:
         size=sizeof(COM_SyncType);
         break;
#if (SYNCCHECK == 1)
      case COM_SYNCCHECK:
         size=sizeof(COM_CheckSyncType);
         break;
#endif
      case COM_SOUNDANDDELTA:
         size=sizeof(MoveType)+sizeof(COM_SoundType);
         break;
      default:
         Error("Unhandled packet type in GetPacketSize type=%ld",((MoveType *)pkt)->type);
         break;
      }

   return size;
}

//****************************************************************************
//
// GetServerPacketSize ()
//
//****************************************************************************

int GetServerPacketSize (void * pkt)
{
   int i;
   byte * ptr;
   COM_ServerHeaderType * serverpkt;

   serverpkt=(COM_ServerHeaderType *)pkt;
   if (serverpkt->type==COM_SERVER)
      {
      ptr=&serverpkt->data;

      for (i=0;i<serverpkt->numpackets;i++)
         {
         ptr+=GetPacketSize(ptr);
         }
      return (ptr-pkt);
      }
   else
      return GetPacketSize(pkt);
}

