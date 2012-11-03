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
//****************************************************************************
//
// _st_menu.h
//
// private header for st_menu.c
//
//****************************************************************************

#ifndef ___st_menu___
#define ___st_menu___

#include "rt_menu.h"

//****************************************************************************
//
// DEFINES
//
//****************************************************************************

#define NOTAVAILABLECOLOR  7
#define NORMALCOLOR        21
#define ACTIVECOLOR        241
#define MAXCURSORNUM       24
#define MAXEDITLIST        14
#define ENTRYX             72
#define ENTRYY             80
#define MANUALX            72
#define MANUALY            105


//****************************************************************************
//
// TYPEDEFS
//
//****************************************************************************

typedef struct {
   char number[15];
   char name[40];
} EditList;



//****************************************************************************
//
// PROTOTYPES
//
//****************************************************************************

void MN_DrawMainMenu (void);
int  MN_HandleMenu (CP_iteminfo *item_i, CP_itemtype *items);
void MN_EraseCursor (CP_iteminfo *item_i, CP_itemtype *items, int x, int y, int which);
void MN_DrawHalfStep (int x, int y);
void MN_DrawCurosr (CP_iteminfo *item_i, CP_itemtype *items, int x, int *y,
                    int which, int basey);
void MN_DisplayInfo (int which);
void MN_DrawMenu (CP_iteminfo *item_i, CP_itemtype *items);
int  MN_GetActive (CP_iteminfo *item_i, CP_itemtype *items, int check, int *nums);
void MN_MakeActive (CP_iteminfo *item_i, CP_itemtype *items, int which);
void MN_WaitKeyUp (void);
void MN_DrawButtons (CP_iteminfo *item_i, CP_itemtype *items, int check, int *nums);
void MN_DrawTitle (int texturenum);

void MN_ModemMenu (void);
void MN_StuffMenu (void);
void MN_MusicMenu (void);
void MN_FXMenu (void);
void MN_MCports (void);

void MN_DrawFXMenu (void);
void MN_DrawMCMenu (void);

void MN_FXSetMenu (void);
void MN_DrawFXCMenu1 (void);
void MN_DrawFXCMenu2 (void);
void MN_DrawFXCMenu3 (void);

void MN_SerialMenu (void);
void MN_DrawSerialMenu (void);

void MN_EditListMenu (void);
void MN_DrawEditListMenu (void);
void MN_PrintEditEntry (int which);
void MN_HandleEntryList (int which);

void MN_DrawTBox (int x, int y, int w, int h, boolean up);

void MN_DrawModemSetupMenu (void);
void MN_ModemSetupMenu (void);

void MN_DrawManualEntry (void);
void MN_ManualEntry (void);

void MN_DrawMusicSoundMenu (void);
void MN_MusicSoundMenu (void);
void MN_DrawVolume (boolean music);
void MN_MusicVolume (void);
void MN_FXVolume (void);



void MN_PlayMenuSnd (int which);

//****************************************************************************
//
// LOCALS
//
//****************************************************************************


CP_iteminfo STMainItems = {44, 65, 5, 0, 28};

CP_itemtype STMainMenu[] =
{
   {2, "mserial\0", 'S', MN_SerialMenu},
   {1, "mmodem\0",  'M', MN_ModemMenu},
   {1, "mmussnd\0", 'M', MN_MusicSoundMenu},
   {1, "mstuff\0",  'S', MN_StuffMenu},
   {1, "mquit\0",   'Q', NULL},
};


CP_iteminfo MusSndItems = {44, 65, 5, 0, 28};

CP_itemtype MusSndMenu[] =
{
   {2, "mmusic\0",  'a', MN_MusicMenu},
   {1, "msndfx\0",  'a', MN_FXMenu},
   {1, "mmusvol\0", 'a', MN_MusicVolume},
   {1, "msndvol\0", 'a', MN_FXVolume},
   {1, "mesc\0",    'a', NULL},
};


CP_iteminfo ModemItems = {44, 65, 5, 0, 28};

CP_itemtype ModemMenu[] =
{
   {2, "mqdial\0", 'Q', NULL},
   {1, "mmdial\0", 'M', MN_ManualEntry},
   {1, "mnlist\0", 'N', MN_EditListMenu},
   {1, "mmsetup\0",'M', MN_ModemSetupMenu},
   {1, "mesc\0",   'E', NULL}
};


CP_iteminfo StuffItems = {44, 65, 4, 0, 48};

CP_itemtype StuffMenu[] =
{
   {2, "mrsnds\0",  'R', NULL},
   {1, "mpgfx\0",   'P', NULL},
   {1, "mblevels\0",'B', NULL},
   {1, "mesc\0",    'E', NULL}
};


CP_iteminfo MusicItems = {44, 65, 8, 0, 48};

CP_itemtype MusicMenu[] =
{
   {2, "mgenmid\0", 'G', MN_MCports},
   {1, "mcanvas\0", 'C', MN_MCports},
   {1, "mwblast\0", 'W', MN_MCports},
   {1, "msblast\0", 'S', MN_MCports},
   {1, "musound\0", 'U', MN_MCports},
   {1, "madlib\0",  'A', MN_MCports},
   {1, "mpas\0",    'P', MN_MCports},
   {1, "mnone\0",   'N', MN_MCports}
};


CP_iteminfo FXItems = {44, 65, 9, 0, 48};

CP_itemtype FXMenu[] =
{
   {2, "musound\0", 'U', MN_FXSetMenu},
   {1, "msblast\0", 'S', MN_FXSetMenu},
   {1, "msndsrc\0", 'S', MN_FXSetMenu},
   {1, "madlib\0",  'A', MN_FXSetMenu},
   {1, "mpcspk\0",  'P', MN_FXSetMenu},
   {1, "mgenmid\0", 'G', MN_FXSetMenu},
   {1, "mpas\0",    'P', MN_FXSetMenu},
   {1, "mwblast\0", 'W', MN_FXSetMenu},
   {1, "mnone\0",   'N', MN_FXSetMenu}
};


CP_iteminfo MPItems = {44, 65, 2, 0, 48};

CP_itemtype MPMenu[] =
{
   {2, "m300\0", 'a', NULL},
   {1, "m330\0", 'a', NULL},
};


CP_iteminfo XItems1 = {44, 65, 8, 0, 48};

CP_itemtype XMenu1 [] =
{
   {1, "m1\0", 'a', NULL},
   {1, "m2\0", 'a', NULL},
   {1, "m3\0", 'a', NULL},
   {2, "m4\0", 'a', NULL},
   {1, "m5\0", 'a', NULL},
   {1, "m6\0", 'a', NULL},
   {1, "m7\0", 'a', NULL},
   {1, "m8\0", 'a', NULL}
};


CP_iteminfo XItems2 = {44, 65, 2, 0, 48};

CP_itemtype XMenu2 [] =
{
   {1, "mmono\0",   'a', NULL},
   {2, "mstereo\0", 'a', NULL}
};


CP_iteminfo XItems3 = {44, 65, 2, 0, 48};

CP_itemtype XMenu3 [] =
{
   {2, "m8\0",  'a', NULL},
   {1, "m16\0", 'a', NULL}
};


CP_iteminfo SerialItems = {44, 65, 5, 0, 28};

CP_itemtype SerialMenu [] =
{
   {2, "mport1\0", 'a', NULL},
   {1, "mport2\0", 'a', NULL},
   {1, "mport3\0", 'a', NULL},
   {1, "mport4\0", 'a', NULL},
   {1, "mesc\0",     'a', NULL}
};


CP_iteminfo PortItems = {44, 65, 4, 0, 48};

CP_itemtype PortMenu [] =
{
   {2, "mport1\0", 'a', NULL},
   {1, "mport2\0", 'a', NULL},
   {1, "mport3\0", 'a', NULL},
   {1, "mport4\0", 'a', NULL}
};


//CP_iteminfo EditItems = {44, 65, 14, 0, 18};
CP_iteminfo EditItems = {50, 65, 14, 0, 12};

CP_itemtype EditMenu [] =
{
   {1,"", 'a', MN_HandleEntryList},
   {1,"", 'b', MN_HandleEntryList},
   {1,"", 'c', MN_HandleEntryList},
   {1,"", 'd', MN_HandleEntryList},
   {1,"", 'e', MN_HandleEntryList},
   {1,"", 'f', MN_HandleEntryList},
   {1,"", 'g', MN_HandleEntryList},
   {1,"", 'h', MN_HandleEntryList},
   {1,"", 'i', MN_HandleEntryList},
   {1,"", 'j', MN_HandleEntryList},
   {1,"", 'k', MN_HandleEntryList},
   {1,"", 'l', MN_HandleEntryList},
   {1,"", 'm', MN_HandleEntryList},
   {1,"", 'n', MN_HandleEntryList}
};

CP_iteminfo ModemSetItems = {44, 65, 6, 0, 28};

CP_itemtype ModemSetMenu [] =
{
   {2, "MSETPORT\0", 'a', NULL},
   {1, "MSETMOD\0",  'a', NULL},
   {1, "MSETBAUD\0", 'a', NULL},
   {1, "MINTSTR\0",  'a', NULL},
   {1, "MANSSTR\0",  'a', NULL},
   {1, "MEXIT\0",    'a', NULL}
};

#endif
