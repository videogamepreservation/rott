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
#ifndef _rt_film_private
#define _rt_film_private

#define MAXEVENTS 100
#define MAXFILMACTORS 30

#define RIGHT 0
#define LEFT 1

typedef enum {
        background,
        backgroundscroll,
        bkgndsprite,
        sprite,
        backdropscroll,
        backdrop,
        palette,
        fadeout
} eventtype;

typedef struct
   {
   int time;
   eventtype type;
   char name[10];
   int length;
   int origheight;
   int x,y,scale;
   int dx,dy,dscale;
   int dir;
   } event;

typedef struct
   {
   int tics;
   int eventnumber;
   int curx,cury,curscale;
   } actortype;

#endif
