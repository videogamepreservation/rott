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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>

#include "rt_def.h"
#include "rt_vid.h"
#include "_rt_vid.h"
#include "rt_menu.h"
#include "rt_util.h"
#include "modexlib.h"
#include "profile.h"
#include "watcom.h"
#include "rt_str.h"
#include "rt_draw.h"
#include "rt_in.h"
#include "rt_main.h"
#include "z_zone.h"
#include "lumpy.h"
#include "rt_vh_a.h"
#include "isr.h"
#include "rt_view.h"
#include "cin_efct.h"
#include "w_wad.h"
//MED
#include "memcheck.h"


//******************************************************************************
//
// GLOBALS
//
//******************************************************************************

byte     *updateptr;
unsigned mapwidthtable[64];
unsigned uwidthtable[UPDATEHIGH];
unsigned blockstarts[UPDATEWIDE*UPDATEHIGH];
byte     update[UPDATESIZE];
byte     palette1[256][3], palette2[256][3];
boolean  screenfaded;


//******************************************************************************
//
// LOCALS
//
//******************************************************************************

static byte  pixmasks[4] = {1,2,4,8};
static byte  leftmasks[4] = {15,14,12,8};
static byte  rightmasks[4] = {1,3,7,15};



//******************************************************************************
//
// VL_MemToScreen ()
//
//******************************************************************************

void VL_MemToScreen (byte *source, int width, int height, int x, int y)
{
   byte *screen, *dest, mask;
   int  plane;

   dest = (byte *)(bufferofs+ylookup[y]+(x>>2));
//   dest = (byte *)(displayofs+ylookup[y]+(x>>2));
   mask = 1 << (x&3);


   for (plane = 0; plane<4; plane++)
   {
      VGAMAPMASK (mask);

      screen = dest;
      for (y = 0; y < height; y++, screen += linewidth, source+=width)
         memcpy (screen, source, width);


      mask <<= 1;

      if (mask == 16)
      {
         mask = 1;
         dest++;
      }
   }
}

//*************************************************************************
//
// DrawTiledRegion () - Fills the specified region with a tiled image
//
//*************************************************************************
void DrawTiledRegion
   (
   int x,
   int y,
   int width,
   int height,
   int offx,
   int offy,
   pic_t *tile
   )

   {
   byte  *source;
   byte  *sourceoff;
   int    sourcex;
   int    sourcey;
   int    sourcewidth;
   int    sourceheight;
   int    mask;
   int    plane;
   int    planesize;
   byte  *start;
   byte  *origdest;
   byte  *dest;
   int    startoffset;
   int    HeightIndex;
   int    WidthIndex;

   start = ( byte * )( bufferofs + ( x>>2 ) + ylookup[ y ] );

   source       = &tile->data;
   sourcewidth  = tile->width;
   sourceheight = tile->height;
   offx >>= 2;
   if ( offx >= sourcewidth )
      {
      offx %= sourcewidth;
      }
   if ( offy >= sourceheight )
      {
      offy %= sourceheight;
      }

   startoffset = offy * sourcewidth;
   planesize = sourcewidth * sourceheight;

   width >>= 2;

   mask  = 1 << ( x & 3 );
   plane = 4;
   while( plane > 0 )
      {
      VGAMAPMASK( mask );
      origdest = start;

      sourcey     = offy;
      sourceoff   = source + startoffset;
      HeightIndex = height;

      while( HeightIndex-- )
         {
         dest       = origdest;
         sourcex    = offx;
         WidthIndex = width;
         while( WidthIndex-- )
            {
            *dest = sourceoff[ sourcex ];
            dest++;
            sourcex++;
            if ( sourcex >= sourcewidth )
               {
               sourcex = 0;
               }
            }

         origdest  += SCREENBWIDE;
         sourceoff += sourcewidth;
         sourcey++;
         if ( sourcey >= sourceheight )
            {
            sourcey   = 0;
            sourceoff = source;
            }
         }

      source += planesize;

      mask <<= 1;
      if ( mask > 8 )
         {
         mask = 1;
         }

      plane--;
      }
   }


//******************************************************************************
//
// VWB_DrawPic () - Draws a linear pic and marks the update block
//
//******************************************************************************

void VWB_DrawPic (int x, int y, pic_t *pic)
{
   if (VW_MarkUpdateBlock (x, y, x+(pic->width<<2)-1, y+(pic->height)-1))
      VL_MemToScreen ((byte *)&pic->data, pic->width, pic->height, x, y);
}



//******************************************************************************
//
// VL_Bar () - Draws a bar
//
//******************************************************************************

void VL_Bar (int x, int y, int width, int height, int color)
{
   byte  *dest;
   byte  leftmask,rightmask;
   int   midbytes,linedelta;

   leftmask = leftmasks[x&3];
   rightmask = rightmasks[(x+width-1)&3];
   midbytes = ((x+width+3)>>2) - (x>>2) - 2;
   linedelta = linewidth-(midbytes+1);

   dest = (byte *)(bufferofs+ylookup[y]+(x>>2));

   if (midbytes < 0)
   {
   // all in one byte
      VGAMAPMASK (leftmask&rightmask);
      while (height--)
      {
         *dest = color;
         dest += linewidth;
      }
      VGAMAPMASK (15);
      return;
   }

   while (height--)
   {
      VGAMAPMASK (leftmask);
      *dest++ = color;

      VGAMAPMASK (15);
      memset (dest,color,midbytes);
      dest += midbytes;

      VGAMAPMASK (rightmask);
      *dest = color;

      dest += linedelta;
   }

   VGAMAPMASK(15);
}



//******************************************************************************
//
// VWB_Bar () - Draws a block and marks the update block
//
//******************************************************************************

void VWB_Bar (int x, int y, int width, int height, int color)
{
   if (VW_MarkUpdateBlock (x,y,x+width,y+height-1) )
      VL_Bar (x, y, width, height, color);
}


//******************************************************************************
//
// VL_TBar () - Draws a bar
//
//******************************************************************************

void VL_TBar (int x, int y, int width, int height)
{
   byte  *dest;
   byte  pixel;
   byte  readmask;
   byte  writemask;
   int   w = width;

   while (height--)
   {
      width = w;

      dest = (byte*)(bufferofs+ylookup[y]+(x>>2));
      readmask    = (x&3);
      writemask   = 1 << readmask;

      VGAREADMAP (readmask);
      VGAMAPMASK (writemask);

      while (width--)
      {
         pixel = *dest;

         pixel = *(colormap+(27<<8)+pixel);

         *dest = pixel;

         writemask <<= 1;
         if (writemask == 16)
         {
            writemask = 1;
            dest++;
         }

         readmask++;
         if (readmask == 4)
            readmask = 0;

         VGAREADMAP (readmask);
         VGAMAPMASK (writemask);
      }

      y++;
   }
}



//******************************************************************************
//
// VWB_TBar () - Draws a block and marks the update block
//
//******************************************************************************

void VWB_TBar (int x, int y, int width, int height)
{
   if (VW_MarkUpdateBlock (x,y,x+width,y+height-1))
      VL_TBar (x, y, width, height);
}


//******************************************************************************
//
// VL_Hlin () - Draws a horizontal line
//
//******************************************************************************

void VL_Hlin (unsigned x, unsigned y, unsigned width, unsigned color)
{
   unsigned xbyte;
   byte     *dest;
   byte     leftmask,
            rightmask;
   int      midbytes;

   xbyte =      x >> 2;
   leftmask    = leftmasks[x&3];
   rightmask   = rightmasks[(x+width-1)&3];
   midbytes    = ((x+width+3)>>2) - xbyte - 2;

   dest = (byte*)(bufferofs+ylookup[y]+xbyte);

   if (midbytes<0)
   {
     // all in one byte
      VGAMAPMASK (leftmask & rightmask);
      *dest = color;
      VGAMAPMASK(15);
      return;
   }

   VGAMAPMASK (leftmask);
   *dest++ = color;

   VGAMAPMASK (15);
   memset (dest, color, midbytes);
   dest += midbytes;

   VGAMAPMASK (rightmask);
   *dest = color;

   VGAMAPMASK (15);
}


//******************************************************************************
//
// VL_Vlin () - Draws a vertical line
//
//******************************************************************************

void VL_Vlin (int x, int y, int height, int color)
{
   byte  *dest,
         mask;

   mask = pixmasks[x&3];
   VGAMAPMASK (mask);

   dest = (byte *)(bufferofs+ylookup[y]+(x>>2));

   while (height--)
   {
      *dest = color;
      dest += linewidth;
   }

   VGAMAPMASK (15);
}



//******************************************************************************
//
// VWB_Hlin () - Draws a horizontal line and marks the update block
//
//******************************************************************************

void VWB_Hlin (int x1, int x2, int y, int color)
{
   if (VW_MarkUpdateBlock (x1,y,x2,y))
      VW_Hlin(x1,x2,y,color);
}


//******************************************************************************
//
// VWB_Vlin () - Draws a vertical line and marks the update block
//
//******************************************************************************

void VWB_Vlin (int y1, int y2, int x, int color)
{
   if (VW_MarkUpdateBlock (x,y1,x,y2))
      VW_Vlin(y1,y2,x,color);
}




//******************************************************************************
//
// VL_THlin ()
//
//******************************************************************************

void VL_THlin (unsigned x, unsigned y, unsigned width, boolean up)
{
   byte     *dest;
   byte     pixel;
   byte     readmask;
   byte     writemask;


   readmask    = (x&3);
   writemask   = 1 << readmask;

   dest = (byte*)(bufferofs+ylookup[y]+(x>>2));

   VGAREADMAP (readmask);
   VGAMAPMASK (writemask);

   while (width--)
   {
      pixel = *dest;

      if (up)
         pixel = *(colormap+(13<<8)+pixel);
      else
         pixel = *(colormap+(27<<8)+pixel);

      *dest = pixel;

      writemask <<= 1;
      if (writemask == 16)
      {
         writemask = 1;
         dest++;
      }

      readmask++;
      if (readmask == 4)
         readmask = 0;

      VGAREADMAP (readmask);
      VGAMAPMASK (writemask);
   }
}



//******************************************************************************
//
// VL_TVlin ()
//
//******************************************************************************

void VL_TVlin (unsigned x, unsigned y, unsigned height, boolean up)
{
   byte     *dest;
   byte     pixel;
   byte     readmask;
   byte     writemask;



   readmask    = (x&3);
   writemask   = 1 << readmask;

   dest = (byte*)(bufferofs+ylookup[y]+(x>>2));

   VGAREADMAP (readmask);
   VGAMAPMASK (writemask);

   while (height--)
   {
      pixel = *dest;

      if (up)
         pixel = *(colormap+(13<<8)+pixel);
      else
         pixel = *(colormap+(27<<8)+pixel);

      *dest = pixel;

      dest += linewidth;
   }
}



//******************************************************************************
//
// VWB_THlin () - Draws a horizontal line and marks the update block
//
//******************************************************************************

void VWB_THlin (int x1, int x2, int y, boolean up)
{
   if (VW_MarkUpdateBlock (x1,y,x2,y))
      VW_THlin (x1,x2,y,up);
}


//******************************************************************************
//
// VWB_TVlin () - Draws a vertical line and marks the update block
//
//******************************************************************************

void VWB_TVlin (int y1, int y2, int x, boolean up)
{
   if (VW_MarkUpdateBlock (x,y1,x,y2))
      VW_TVlin (y1,y2,x,up);
}



/*
================================================================================

            Double buffer management routines

================================================================================
*/


//******************************************************************************
//
// VW_MarkUpdateBlock
//
// Takes a pixel bounded block and marks the tiles in bufferblocks
// Returns 0 if the entire block is off the buffer screen
//
//******************************************************************************

int VW_MarkUpdateBlock (int x1, int y1, int x2, int y2)
{
   int   x,
         y,
         xt1,
         yt1,
         xt2,
         yt2,
         nextline;
   byte  *mark;

   xt1 = x1 >> PIXTOBLOCK;
   yt1 = y1 >> PIXTOBLOCK;

   xt2 = x2 >> PIXTOBLOCK;
   yt2 = y2 >> PIXTOBLOCK;

   if (xt1 < 0)
      xt1 = 0;
   else
      if (xt1 >= UPDATEWIDE)
         return 0;

   if (yt1 < 0)
      yt1 = 0;
   else
      if (yt1 > UPDATEHIGH)
         return 0;

   if (xt2 < 0)
      return 0;
   else
      if (xt2 >= UPDATEWIDE)
         xt2 = UPDATEWIDE-1;

   if (yt2 < 0)
      return 0;
   else
      if (yt2 >= UPDATEHIGH)
         yt2 = UPDATEHIGH-1;

   mark = updateptr + uwidthtable[yt1] + xt1;
   nextline = UPDATEWIDE - (xt2-xt1) - 1;

   for (y = yt1; y <= yt2; y++)
   {
      for (x = xt1; x <= xt2; x++)
         *mark++ = 1;                  // this tile will need to be updated

      mark += nextline;
   }

   return 1;
}


//******************************************************************************
//
// VW_UpdateScreen ()
//
//******************************************************************************


void VW_UpdateScreen (void)
{
   VH_UpdateScreen ();
}



/*
=============================================================================

                  PALETTE OPS

      To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (int red, int green, int blue)
{
   int   i;

   OUTP (PEL_WRITE_ADR,0);
   for (i=0;i<256;i++)
   {
      OUTP (PEL_DATA,red);
      OUTP (PEL_DATA,green);
      OUTP (PEL_DATA,blue);
   }
}

//===========================================================================

/*
=================
=
= VL_SetColor
=
=================
*/

void VL_SetColor  (int color, int red, int green, int blue)
{
   OUTP (PEL_WRITE_ADR,color);
   OUTP (PEL_DATA,red);
   OUTP (PEL_DATA,green);
   OUTP (PEL_DATA,blue);
}

//===========================================================================

/*
=================
=
= VL_GetColor
=
=================
*/

void VL_GetColor  (int color, int *red, int *green, int *blue)
{
   OUTP (PEL_READ_ADR,color);
   *red   = inp (PEL_DATA);
   *green = inp (PEL_DATA);
   *blue  = inp (PEL_DATA);
}

//===========================================================================

//===========================================================================

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (int start, int end, int red, int green, int blue, int steps)
{
   int      i,j,orig,delta;
   byte  *origptr, *newptr;

   if (screenfaded)
      return;

   WaitVBL ();
   VL_GetPalette (&palette1[0][0]);
   memcpy (palette2, palette1, 768);

//
// fade through intermediate frames
//
   for (i = 0; i < steps; i++)
   {
      origptr = &palette1[start][0];
      newptr = &palette2[start][0];

      for (j = start; j <= end; j++)
      {
         orig = *origptr++;
         delta = red-orig;
         *newptr++ = orig + delta * i / steps;
         orig = *origptr++;
         delta = green-orig;
         *newptr++ = orig + delta * i / steps;
         orig = *origptr++;
         delta = blue-orig;
         *newptr++ = orig + delta * i / steps;
      }

      WaitVBL ();
      VL_SetPalette (&palette2[0][0]);
   }

//
// final color
//
   VL_FillPalette (red,green,blue);

   screenfaded = true;
}


/*
=================
=
= VL_FadeToColor
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeToColor (int time, int red, int green, int blue)
{
   int      i,j,orig,delta;
   byte  *origptr, *newptr;
   int dmax,dmin;

   if (screenfaded)
      return;

   WaitVBL ();
   VL_GetPalette (&palette1[0][0]);
   memcpy (palette2, palette1, 768);

   dmax=(maxshade<<16)/time;
   dmin=(minshade<<16)/time;
//
// fade through intermediate frames
//
   for (i = 0; i < time; i+=tics)
   {
      origptr = &palette1[0][0];
      newptr = &palette2[0][0];

      for (j = 0; j <= 255; j++)
      {
         orig = *origptr++;
         delta = ((red>>2)-orig)<<16;
         *newptr++ = orig + FixedMul(delta/time,i);
         orig = *origptr++;
         delta = ((green>>2)-orig)<<16;
         *newptr++ = orig + FixedMul(delta/time,i);
         orig = *origptr++;
         delta = ((blue>>2)-orig)<<16;
         *newptr++ = orig + FixedMul(delta/time,i);
      }

      maxshade=(dmax*(time-i))>>16;
      minshade=(dmin*(time-i))>>16;
      WaitVBL ();
      VL_SetPalette (&palette2[0][0]);
      ThreeDRefresh();
      CalcTics();

   }

//
// final color
//
   VL_FillPalette (red>>2,green>>2,blue>>2);

   screenfaded = true;
}




/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (int start, int end, byte *palette, int steps)
{
   int      i,j,delta;

   WaitVBL ();
   VL_GetPalette (&palette1[0][0]);

   memcpy (&palette2[0][0], &palette1[0][0], sizeof(palette1));

   start *= 3;
   end = end*3+2;

//
// fade through intermediate frames
//
   for (i=0;i<steps;i++)
   {
      for (j=start;j<=end;j++)
      {
         delta = palette[j]-palette1[0][j];
         palette2[0][j] = palette1[0][j] + delta * i / steps;
      }

      WaitVBL ();
      VL_SetPalette (&palette2[0][0]);
   }

//
// final color
//
   VL_SetPalette (palette);
   screenfaded = false;
}



//******************************************************************************
//
// SwitchPalette
//
//******************************************************************************

void SwitchPalette (byte * newpal, int steps)
{
   int temp;

   VL_FadeOut(0,255,0,0,0,steps>>1);

   temp = bufferofs;
   bufferofs = displayofs;
   VL_Bar (0, 0, 320, 200, 0);
   bufferofs = temp;

   VL_FadeIn(0,255,newpal,steps>>1);
}


#if 0

/*
=================
=
= VL_TestPaletteSet
=
= Sets the palette with outsb, then reads it in and compares
= If it compares ok, fastpalette is set to true.
=
=================
*/

void VL_TestPaletteSet (void)
{
   int   i;

   for (i=0;i<768;i++)
      palette1[0][i] = i;

   fastpalette = true;
   VL_SetPalette (&palette1[0][0]);
   VL_GetPalette (&palette2[0][0]);
   if (_fmemcmp (&palette1[0][0],&palette2[0][0],768))
      fastpalette = false;
}


/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (int color)
{
   _AH=0x10;
   _AL=1;
   _BH=color;
   geninterrupt (0x10);
   bordercolor = color;
}


#endif


//==========================================================================

//****************************************************************************
//
// VL_DecompressLBM ()
//
// LIMITATIONS - Only works with 320x200!!!
//
//****************************************************************************

void VL_DecompressLBM (lbm_t *lbminfo, boolean flip)
{
   byte *screen = (byte *)bufferofs;
   byte *orig;
	int  count;
	byte b,
        rept;
   byte *source = (byte *)&lbminfo->data;
   byte *buf;
   int  ht = lbminfo->height;
   int  planes;
   byte writemask;
   int  x = 0;
   int  y;
   byte *origbuf;
   byte pal[768];

   writemask   = 1 << (x&3);
   orig = screen;

   buf = (byte *) SafeMalloc (64000);

   origbuf = buf;

   VL_ClearBuffer (displayofs, 0);

   memcpy(&pal[0],lbminfo->palette,768);

   VL_NormalizePalette (&pal[0]);

   VW_MarkUpdateBlock (0, 0, 320, 200);

   while (ht--)
   {
      count = 0;

   	do
	   {
		   rept = *source++;

   		if (rept > 0x80)
	   	{
		   	rept = (rept^0xff)+2;
			   b = *source++;
   			memset (buf, b, rept);
	   		buf += rept;
		   }
   		else if (rept < 0x80)
	   	{
		   	rept++;
			   memcpy (buf, source, rept);
   			buf += rept;
	   		source += rept;
		   }
   		else
	   		rept = 0;               // rept of 0x80 is NOP

		   count += rept;

   	} while (count < lbminfo->width);
   }

   for (planes = 0; planes < 4; planes++)
   {
      int cnt;

      cnt = 0;
      screen = orig;
      buf = origbuf;
      VGAMAPMASK (writemask);

      for (y = 0; y < ((lbminfo->height*lbminfo->width)>>2); y++)
      {
         *screen++ = *(buf+(y*4)+planes);
         cnt++;

         if (cnt == 80)
         {
            screen += 16;
            cnt = 0;
         }
      }

      writemask <<= 1;
   }

   SafeFree(origbuf);

   if (flip==true)
      VW_UpdateScreen ();

   VL_FadeIn (0, 255, &pal[0], 15);

}

//****************************************************************************
//
// SetBorderColor
//
//****************************************************************************

void SetBorderColor (int color)
{
   inp  (STATUS_REGISTER_1);
   outp (ATR_INDEX,0x31);
   outp (ATR_INDEX,color);
}

//****************************************************************************
//
// SetBorderColorInterrupt
//
//****************************************************************************

void SetBorderColorInterrupt (int color)
{
   union REGS regs;

   regs.w.ax = 0x1001;
   regs.w.bx = color<<8;
   int386(0x10,&regs,&regs);
}


//****************************************************************************
//
// VL_DrawPostPic
//
//****************************************************************************

void VL_DrawPostPic (int lumpnum)
{
   DrawPostPic(lumpnum);
   VW_MarkUpdateBlock (0, 0, 319, 199);
}

//****************************************************************************
//
// VL_DrawLine
//
//****************************************************************************

void VL_DrawLine (int x1, int y1, int x2, int y2, byte color)
{
   int dx;
   int dy;
   int xinc;
   int yinc;
   int count;

   dx=(x2-x1);
   dy=(y2-y1);
   if (abs(dy)>=abs(dx))
      {
      count=abs(dy);
      yinc=(dy<<16)/count;
      if (dy==0)
         {
         return;
         }
      else
         {
         xinc=(dx<<16)/count;
         }
      }
   else
      {
      count=abs(dx);
      xinc=(dx<<16)/count;
      if (dx==0)
         {
         return;
         }
      else
         {
         yinc=(dy<<16)/count;
         }
      }
   x1<<=16;
   y1<<=16;
   while (count>0)
      {
      VGAWRITEMAP((x1>>16)&3);
      *((byte *)bufferofs+(x1>>18)+(ylookup[y1>>16]))=color;
      x1+=xinc;
      y1+=yinc;
      count--;
      }
}


//******************************************************************************
//
// DrawXYPic
//
//******************************************************************************

void DrawXYPic (int x, int y, int shapenum)
{
   byte *buffer;
   byte *buf;
   int xx,yy;
   int plane;
   byte *src;
   pic_t *p;

   p = (pic_t *) W_CacheLumpNum (shapenum, PU_CACHE);

   if ((x<0) || ((x+(p->width<<2))>=320))
      Error ("DrawXYPic: x is out of range\n");
   if ((y<0) || ((y+p->height)>=200))
      Error ("DrawXYPic: y is out of range\n");


   buffer = (byte*)bufferofs+(x>>2)+ylookup[y];

   src=(byte *)&p->data;
   for (plane=x;plane<x+4;plane++)
      {
      VGAWRITEMAP((plane&3));
      for (yy = 0; yy < p->height; yy++)
         {
         buf=buffer+ylookup[yy];
         for (xx = 0; xx < p->width; xx++,buf++)
            *(buf)=*(src++);
         }
      }
}


