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
// Public header for RT_STR.C.
//
//***************************************************************************

#ifndef _rt_str_public
#define _rt_str_public

#include "lumpy.h"


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern int fontcolor;


//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************

typedef  struct
{
   int   x,y;
} Point;

typedef  struct
{
   int   x, y,
         w, h,
         px, py;
} WindowRec;            // Record used to save & restore screen windows

typedef  struct
{
	Point ul,lr;
} Rect;


//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

//
// String rtns
//

void VW_DrawClippedString (int x, int y, char *string);
void US_ClippedPrint (int x, int y, char *s);

void VWB_DrawPropString  (char *string);
void VW_MeasurePropString (char *string, int *width, int *height);

void US_MeasureStr (int *width, int *height, char * s, ...);

void VW_DrawPropString (char *string);

void US_SetPrintRoutines (void (*measure)(char *, int *, int *, font_t *),
                          void (*print)(char *));
void US_Print (char *s);
void US_BufPrint (char *s);
void US_PrintUnsigned (unsigned long int n);
void US_PrintSigned (long int n);
void USL_PrintInCenter (char *s, Rect r);
void US_PrintCentered (char *s);
void US_CPrintLine (char *s);
void US_CPrint (char *s);


//
// Input rtns
//

boolean US_LineInput (int x, int y, char *buf, char *def, boolean escok,
                      int maxchars, int maxwidth, int color);
boolean US_lineinput (int x, int y, char *buf, char *def, boolean escok,
                      int maxchars, int maxwidth, int color);
int CalibrateJoystick(void);

//
// Window rtns
//

void US_DrawWindow (int x, int y, int w, int h);
void US_CenterWindow (int w, int h);

//
// Intensity font rtns
//

void DrawIString (unsigned short int x, unsigned short int y, char *string, int flags);
void DrawIntensityString (unsigned short int x, unsigned short int y, char *string, int color);
void VW_MeasureIntensityPropString (char *string, int *width, int *height);
byte GetIntensityColor (byte pix);

#include "myprint.h"

#endif
