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
#ifndef _rt_def_public
#define _rt_def_public


// RT_DEF.H Zee big one
#include <stdio.h>
#include "develop.h"
#define SAVE_SCREEN  1

//
//
//
//***************************************************************************
//
//    Global Constants
//
//***************************************************************************

#define PI      3.141592657
#define NULL       0
#define LONG(a) ((int)a)
#define M_PI            3.14159

//***************************************************************************
//
//    Screen Constants
//
//***************************************************************************
#define VIEWGLOBAL              0x10000         // globals visable flush to wall
#define VIEWWIDTH               320             // size of view window
#define VIEWHEIGHT              200
#define MAXSCANLINES            200             // size of ylookup table
#define CHARWIDTH               2
#define TILEWIDTH               4
#define STATUSLINES             16

//***************************************************************************
//
//    Engine Constants
//
//***************************************************************************

#define ANGLES                            2048 // must be divisible by 4
#define ANGLEQUAD                         (ANGLES/4)
#define FINEANGLES                        2048
#define FINEANGLEQUAD                     (FINEANGLES/4)
#define ANG90                             (FINEANGLES/4)
#define ANG180                            (ANG90*2)
#define ANG270                            (ANG90*3)
#define ANG360                            (ANG90*4)
#define VANG90                            (ANGLES/4)
#define VANG180                           (VANG90*2)
#define VANG270                           (VANG90*3)
#define VANG360                           (VANG90*4)
#define ANGLESDIV8                        (ANGLES/8)
#define MINDIST                           (0x5800l)
#define PIXRADIUS                         512
#define FOCALLENGTH                       (0x5700l)             // in global coordinates
#define MAXTICS                           10

//***************************************************************************
//
//    Map Constants/ macros
//
//***************************************************************************


#define GLOBAL1                           (1l<<16)
#define TILEGLOBAL                        GLOBAL1
#define PIXGLOBAL                         (GLOBAL1/64)
#define TILESHIFT                         16l
#define UNSIGNEDSHIFT                     8
#define PLAYERSIZE                        0x5700l                       // player radius
#define MAPSIZE                           128                         // maps are 64*64 max
#define NUMAREAS                          47
#define MAPSPOT(x,y,plane)                (mapplanes[plane][MAPSIZE*(y)+(x)])
#define AREATILE                          107
#define ICONARROWS                        72
#define PUSHABLETILE                      80
#define ELEVATORTILE                      72
#define ALTELEVATORTILE                   106
#define LASTLEVELVALUE                    459

#define AREANUMBER(x,y)                   (MAPSPOT((x),(y),0)-AREATILE)

//***************************************************************************
//
//    Scale Constants
//
//***************************************************************************

#define SFRACBITS 16
#define SFRACUNIT (0x10000)
#define FRACUNIT (0x100)

//***************************************************************************
//
//    Actor Constants
//
//***************************************************************************

#define RUNSPEED                6000
#define MINACTORDIST            0x9000l
#define MAXSTATS                400                           // max number of lamps, bonus, etc

#define MAXWALLTILES            105          // max number of wall tiles
#define MAXACTORS               600
#define NORTH                   0
#define EAST                    1
#define SOUTH                   2
#define WEST                    3

//***************************************************************************
//
//    Input Defines - joystick, keyboard and mouse
//
//***************************************************************************

#define MaxJoys            2
#define MaxKbds            2
#define MaxJoys            2
#define NumCodes           128
#define MReset             0
#define MButtons           3
#define MDelta             11

// Key definitions

#define  key_None    0
#define  key_Return     0x0d
#define  key_Enter      key_Return
#define  key_Escape     0x1b
#define  key_Space      0x20
#define  key_BackSpace  0x08
#define  key_Tab        0x09
#define  key_Delete     0x7f

#define  ANGLEBITS      16
#define  ANGLEFRACMAX   (FINEANGLES<<ANGLEBITS)


//***************************************************************************
//
//    SWIFT Constants
//
//***************************************************************************

//
// device type codes, returned in deviceType field (SWIFT_StaticData)
//
#define SWIFT_DEV_NONE		0
#define SWIFT_DEV_CYBERMAN	1

//
// Dynamic device data
//
#define SDD_EXTERNAL_POWER_CONNECTED	1
#define SDD_EXTERNAL_POWER_TOO_HIGH	   2

#define AX(r) ((r).x.eax)
#define BX(r) ((r).x.ebx)
#define CX(r) ((r).x.ecx)
#define DX(r) ((r).x.edx)
#define SI(r) ((r).x.esi)
#define DI(r) ((r).x.edi)

//***************************************************************************
//
//    Global Types
//
//***************************************************************************

///////////////////      GLOBAL DATA TYPES ///////////////////////////////


typedef unsigned char           byte;
typedef unsigned short int      word;
typedef unsigned long           longword;
typedef long fixed;


//////////////////////////////////////////////////////////////////////////


//////////////////      GLOBAL ENUMERATED TYPES    ///////////////////////


typedef enum
 {false,
  true
 }
 boolean;



typedef enum {
		  east,
		  northeast,
		  north,
		  northwest,
		  west,
		  southwest,
		  south,
		  southeast,
		  nodir
} dirtype;

typedef enum
 {SPRITE,
  WALL,
  ACTOR,
  DOOR,
  PWALL,
  MWALL
 }
 thingtype;

#define NUMTXBUTTONS    16
typedef enum    {
		  bt_nobutton=-1,
		  bt_attack=0,
		  bt_strafe=1,
		  bt_run=2,
		  bt_use=3,
		  bt_lookup=4,
		  bt_lookdown=5,
		  bt_swapweapon=6,
		  bt_dropweapon=7,
		  bt_horizonup=8,
        bt_horizondown=9,
        bt_pistol=10,
        bt_dualpistol=11,
        bt_mp40=12,
        bt_missileweapon=13,
        bt_autorun=14,
		  bt_recordsound=15,
		  bt_strafeleft=16,
		  bt_straferight=17,
        bt_turnaround=18,
        bt_aimbutton=19,
        di_north=20,
        di_east=21,
        di_south=22,
        di_west=23,
        bt_map=24,
        bt_message=25,
        bt_directmsg=26,
        NUMBUTTONS
};


#if (SHAREWARE == 0)
 #define MAXWEAPONS  13
#else
 #define MAXWEAPONS  9
#endif


typedef enum    { wp_pistol,
						wp_twopistol,
						wp_mp40,
						wp_bazooka,
						wp_heatseeker,
                  wp_drunk,
                  wp_firebomb,
                  wp_firewall,
						wp_godhand,

#if (SHAREWARE == 0)
                  wp_split,
						wp_kes,
						wp_bat,
                  wp_dog
#endif

                } weapontype;



typedef enum    {
						gd_baby,
						gd_easy,
						gd_medium,
						gd_hard
};


typedef enum    {
		  ex_stillplaying,
		  ex_completed,
		  ex_died,
		  ex_warped,
		  ex_resetgame,
		  ex_loadedgame,
		  ex_victorious,
		  ex_abort,
		  ex_demodone,
		  ex_skiplevel,
		  ex_secretlevel,
        ex_secretdone,
        ex_titles,
		  ex_demorecord,
		  ex_demoplayback,

		  ex_bossdied,
		  ex_gameover,
		  ex_battledone
} exit_t;

////////////////////////////////////////////////////////////////////////////

/////////////////      GLOBAL STRUCTURE TYPES     //////////////////////////


//=================== SHARED FLAGS =====================================//

#define FL_SHOOTABLE            0x01
#define FL_ACTIVE               0x02
#define FL_VISIBLE              0x08
#define FL_NOFRICTION           0x100 //
#define FL_DYING                0x1000
#define FL_ALTERNATE            0x20000
#define FL_GODSTRUCK            0x80000
#define FL_ABP                  0x400000
#define FL_BLOCK                0x800000
#define FL_HBM                  0x1000000
#define FL_RIDING               0x2000000
#define FL_SOLIDCOLOR           0x4000000
#define FL_SEEN                 0x8000000
#define FL_COLORED              0x10000000
#define FL_FULLLIGHT       0x80000000

//=================== ACTOR FLAGS =====================================//

#define FL_NEVERMARK            4
#define FL_ATTACKMODE           0x10
#define FL_FIRSTATTACK          0x20
#define FL_ISFIRE               0x20
#define FL_AMBUSH               0x40
#define FL_NONMARK              0x80
#define FL_DONE                 0x200 // used by push column
#define FL_DODGE                0x400 // "
#define FL_STUCK                0x800 //
#define FL_HASAUTO              0x2000
#define FL_FALLINGOBJECT        0x4000
#define FL_KEYACTOR             0x8000
#define FL_HEAD                 0x40000
#define FL_TARGET               0x200000
#define FL_FLIPPED              0x40000000
#define FL_CRAZY                0x40000000


//================== PLAYER FLAGS ======================================//

#define FL_BPV                  0x10
#define FL_SHROOMS              0x20
#define FL_AV                   0x40
#define FL_ELASTO               0x200
#define FL_FLEET                0x400
#define FL_PUSHED               0x2000
#define FL_PAIN                 0x4000
#define FL_GODMODE              0x8000
#define FL_DOGMODE              0x10000
#define FL_GASMASK              0x100000
#define FL_DESIGNATED           0x20000000
#define FL_DIDTAG               0x40000000

//================== SPRITE FLAGS ======================================//

#define NOTHING                 -1
#define FL_BONUS                0x04
#define FL_LIGHT                0x20
#define FL_CHANGES              0x40
#define FL_BACKWARDS            0x100
#define FL_BANDF                0x200
#define FL_HEAT                 0x400
#define FL_LIGHTOFF             0x800
#define FL_ACTOR                0x1000
#define FL_ROTATING             0x2000
#define FL_RESPAWN              0x4000
#define FL_WOODEN               0x8000
#define FL_METALLIC             0x10000
#define FL_EARTHEN              0x20000
#define FL_LIGHTON              0x40000
#define FL_FADING               0x80000
#define FL_DEADBODY             0x100000
#define FL_WEAPON               0x200000
#define FL_TRANSLUCENT          0x2000000
#define FL_HEIGHTFLIPPABLE      0x40000000

#endif
