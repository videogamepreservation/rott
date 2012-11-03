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
#ifndef _lumpy_
#define _lumpy_

//****************************************************************************
//
// Public header for LUMPY typedefs
//
//****************************************************************************


typedef struct
{
   byte     width,height;
   byte     data;
} pic_t;

typedef struct
{
   short     width,height;
   short     orgx,orgy;
   byte     data;
} lpic_t;


typedef struct
{
	short height;
   char  width[256];
   short charofs[256];
   byte  data;       // as much as required
} font_t;


typedef struct
{
   short width;
   short height;
   byte palette[768];
   byte data;
} lbm_t;


typedef struct
{
   short          origsize;         // the orig size of "grabbed" gfx
   short          width;            // bounding box size
   short          height;
   short          leftoffset;       // pixels to the left of origin
   short          topoffset;        // pixels above the origin
   unsigned short collumnofs[320];  // only [width] used, the [0] is &collumnofs[width]
} patch_t;


typedef struct
{
   short origsize;         // the orig size of "grabbed" gfx
   short width;            // bounding box size
   short height;
   short leftoffset;       // pixels to the left of origin
   short topoffset;        // pixels above the origin
   short translevel;
   short collumnofs[320];  // only [width] used, the [0] is &collumnofs[width]
} transpatch_t;


typedef struct
{
   byte  color;
   short height;
   char  width[256];
   short charofs[256];
   byte  pal[0x300];
   byte  data;       // as much as required
} cfont_t;

#endif
