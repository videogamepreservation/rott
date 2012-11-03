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
#include <malloc.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <string.h>
#include "rt_def.h"
#include "scriplib.h"
#include "watcom.h"
#include "f_scale.h"
#include "_rt_film.h"
#include "rt_scale.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_in.h"
#include "rt_film.h"
#include "modexlib.h"
#include "rt_util.h"
#include "rt_vid.h"
#include "lumpy.h"
//MED
#include "memcheck.h"

int dc_ycenter;

static byte * filmbuffer;
static event * events[MAXEVENTS];
static actortype * actors[MAXFILMACTORS];
static int eventindex;
static int currentevent;
static int dtime;
static int movielength;
static int filmtics;
static int lastfilmactor;
static byte firsttime=1;


/*
================
=
= FirstTimeInitialize
=
================
*/
void FirstTimeInitialize ( void )
{
   if (firsttime==0)
      return;

   memset(actors,NULL,sizeof(actors));
   memset(events,NULL,sizeof(events));

   firsttime=0;
}


/*
================
=
= InitializeFilmActors
=
================
*/
void InitializeFilmActors ( void )
{
   int i;

   for (i=0;i<MAXFILMACTORS;i++)
      {
      if (actors[i]!=NULL)
         SafeFree(actors[i]);
      }
   lastfilmactor=0;
}

/*
================
=
= InitializeEvents
=
================
*/
void InitializeEvents ( void )
{
   int i;

   for (i=0;i<MAXEVENTS;i++)
      {
      if (events[i]!=NULL)
         SafeFree(events[i]);
      }
   currentevent=0;
   eventindex=0;
}

/*
================
=
= CleanupEvents
=
================
*/
void CleanupEvents ( void )
{
   int i;

   for (i=0;i<MAXEVENTS;i++)
      if (events[i]!=NULL)
         {
         SafeFree(events[i]);
         events[i]=NULL;
         }
}

/*
================
=
= CleanupActors
=
================
*/
void CleanupActors ( void )
{
   int i;

   for (i=0;i<MAXFILMACTORS;i++)
      if (actors[i]!=NULL)
         {
         SafeFree(actors[i]);
         actors[i]=NULL;
         }
}

/*
================
=
= GetNewFilmEvent
=
================
*/
event * GetNewFilmEvent ( void )
{
   if (eventindex==MAXEVENTS)
      Error("No free Events in Cinematic engine\n");
   events[eventindex]=SafeMalloc(sizeof(event));
   eventindex++;
   return events[eventindex-1];
}

/*
================
=
= ParseMovieScript
=
================
*/

void ParseMovieScript (void)
{
   int time;
   event * newevent;

   time=0;
   do
      {
      //
      // get a command / lump name
      //
      GetToken (true);
      if (endofscript)
         break;
      newevent=GetNewFilmEvent();
      time+=ParseNum(token);
      GetToken (false);
      if (!strcmpi (token,"BACKGROUND"))
         {
         GetToken (false);
         newevent->time=time;
         strcpy(&(newevent->name[0]),token);
         GetToken (false);
         if (!strcmpi (token,"SCROLLRIGHT"))
            {
            newevent->dir=RIGHT;
            newevent->type=backgroundscroll;
            }
         else if (!strcmpi (token,"SCROLLLEFT"))
            {
            newevent->dir=LEFT;
            newevent->type=backgroundscroll;
            }
         else
            {
            newevent->type=background;
            }
         GetToken (false);
         newevent->dx=ParseNum(token);
         newevent->x=0;
         }
      else if (!strcmpi (token,"BACKDROP"))
         {
         GetToken (false);
         newevent->time=time;
         newevent->type=backdrop;
         strcpy(&(newevent->name[0]),token);
         GetToken (false);
         if (!strcmpi (token,"SCROLLRIGHT"))
            {
            newevent->dir=RIGHT;
            newevent->type=backdropscroll;
            }
         else if (!strcmpi (token,"SCROLLLEFT"))
            {
            newevent->dir=LEFT;
            newevent->type=backdropscroll;
            }
         else
            {
            newevent->type=backdrop;
            }
         GetToken (false);
         newevent->dx=ParseNum(token);
         newevent->x=0;
         }
      else if (!strcmpi (token,"BKGNDSPRITE"))
         {
         GetToken (false);
         newevent->time=time;
         newevent->type=bkgndsprite;
         strcpy(&(newevent->name[0]),token);
         GetToken (false);
         newevent->length=ParseNum(token);
         GetToken (false);
         newevent->origheight=ParseNum(token);
         GetToken (false);
         newevent->x=ParseNum(token);
         GetToken (false);
         newevent->y=ParseNum(token);
         GetToken (false);
         newevent->scale=ParseNum(token);
         GetToken (false);
         newevent->dx=ParseNum(token);
         GetToken (false);
         newevent->dy=ParseNum(token);
         GetToken (false);
         newevent->dscale=ParseNum(token);
         }
      else if (!strcmpi (token,"SPRITE"))
         {
         GetToken (false);
         newevent->time=time;
         newevent->type=sprite;
         strcpy(&(newevent->name[0]),token);
         GetToken (false);
         newevent->length=ParseNum(token);
         GetToken (false);
         newevent->origheight=ParseNum(token);
         GetToken (false);
         newevent->x=ParseNum(token);
         GetToken (false);
         newevent->y=ParseNum(token);
         GetToken (false);
         newevent->scale=ParseNum(token);
         GetToken (false);
         newevent->dx=ParseNum(token);
         GetToken (false);
         newevent->dy=ParseNum(token);
         GetToken (false);
         newevent->dscale=ParseNum(token);
         }
      else if (!strcmpi (token,"PALETTE"))
         {
         GetToken (false);
         newevent->time=time;
         newevent->type=palette;
         strcpy(&(newevent->name[0]),token);
         }
      else if (!strcmpi (token,"FADEOUT"))
         {
         newevent->time=time;
         newevent->type=fadeout;
         GetToken (false);
         newevent->length=ParseNum(token);
         }
      else if (!strcmpi (token,"MOVIEEND"))
         {
         movielength=time;
         eventindex--;
         }
      else
         Error("PARSE: Illegal Token %s\n",token);
//      eventindex++;
      }
   while (script_p < scriptend_p);
   if (movielength==0)
      Error("No end of movie marker\n");
}


/*
=================
=
= DumpMovie
=
=================
*/
void DumpMovie( void )
{
   int i;
   char typestring[45];

   if (eventindex==0)
      Error ("No events in movie");
   for (i=0;i<eventindex;i++)
      {
      printf("Event #%ld at %ld tics\n",i,events[i]->time);
      switch (events[i]->type)
         {
         case background:
            strcpy(&typestring[0],"BACKGROUND");
            break;
         case backdrop:
            strcpy(&typestring[0],"BACKDROP");
            break;
         case bkgndsprite:
            strcpy(&typestring[0],"BKGNDSPRITE");
            break;
         case sprite:
            strcpy(&typestring[0],"SPRITE");
            break;
         case palette:
            strcpy(&typestring[0],"PALLETE");
            break;
         default:
            Error("DUMP: Illegal event type\n");
            break;
         }
      printf("%s\n",typestring);
      switch (events[i]->type)
         {
         case background:
            printf("   NAME: %s\n",events[i]->name);
            break;
         case backdrop:
            printf("   NAME: %s\n",events[i]->name);
            break;
         case palette:
            printf("   NAME: %s\n",events[i]->name);
            break;
         case bkgndsprite:
            printf("   NAME: %s\n",events[i]->name);
            if (events[i]->length!=-1)
               printf(" LENGTH: %ld\n",events[i]->length);
            else
               printf(" LENGTH: INFINITE\n");
            printf("ORGHGHT: %ld\n",events[i]->origheight);
            printf("      X: %ld\n",events[i]->x);
            printf("      Y: %ld\n",events[i]->y);
            printf("  SCALE: %ld\n",events[i]->scale);
            if (events[i]->dx)
               printf("     DX: %ld\n",events[i]->dx);
            else
               printf("     DX: STATIC\n");
            if (events[i]->dy)
               printf("     DY: %ld\n",events[i]->dy);
            else
               printf("     DY: STATIC\n");
            if (events[i]->dscale)
               printf(" DSCALE: %ld\n",events[i]->dscale);
            else
               printf(" DSCALE: STATIC\n");
            break;
         case sprite:
            printf("   NAME: %s\n",events[i]->name);
            if (events[i]->length!=-1)
               printf(" LENGTH: %ld\n",events[i]->length);
            else
               printf(" LENGTH: INFINITE\n");
            printf("ORGHGHT: %ld\n",events[i]->origheight);
            printf("      X: %ld\n",events[i]->x);
            printf("      Y: %ld\n",events[i]->y);
            printf("  SCALE: %ld\n",events[i]->scale);
            if (events[i]->dx)
               printf("     DX: %ld\n",events[i]->dx);
            else
               printf("     DX: STATIC\n");
            if (events[i]->dy)
               printf("     DY: %ld\n",events[i]->dy);
            else
               printf("     DY: STATIC\n");
            if (events[i]->dscale)
               printf(" DSCALE: %ld\n",events[i]->dscale);
            else
               printf(" DSCALE: STATIC\n");
            break;
         default:
            Error("DUMP: Illegal event type\n");
            break;
         }
      }
}

/*
=================
=
= GetFreeActor
=
=================
*/
actortype * GetFreeActor ( void )
{
   int i;

   for (i=0;i<MAXFILMACTORS;i++)
      if (actors[i]==NULL)
         {
         actors[i]=SafeMalloc(sizeof(actortype));
         if (i>lastfilmactor)
            lastfilmactor=i;
         return actors[i];
         }
   Error ("No free spots in actorlist");
   return NULL;
}


/*
=================
=
= FreeActor
=
=================
*/
void FreeActor ( int i )
{
   SafeFree(actors[i]);
   actors[i]=NULL;
}

/*
=================
=
= UpdateEvents
=
=================
*/
void UpdateEvents ( void )
{
   int i,j;

   for (i=0;i<=lastfilmactor;)
      {
      if (actors[i]!=NULL)
         {
         for (j=0;j<dtime;j++)
            {
            if (actors[i]->tics!=-1)
               {
               actors[i]->tics--;
               if (actors[i]->tics<=0)
                  {
                  FreeActor(i);
                  goto continueloop;
                  }
               }
            switch (events[actors[i]->eventnumber]->type)
               {
               case background:
               case backdrop:
               case palette:
               case fadeout:
                  break;
               case backgroundscroll:
               case backdropscroll:
                  if (events[actors[i]->eventnumber]->dir==RIGHT)
                     actors[i]->curx-=events[actors[i]->eventnumber]->dx;
                  else if (events[actors[i]->eventnumber]->dir==LEFT)
                     actors[i]->curx+=events[actors[i]->eventnumber]->dx;
                  break;
               case bkgndsprite:
                  actors[i]->curx+=events[actors[i]->eventnumber]->dx;
                  actors[i]->cury+=events[actors[i]->eventnumber]->dy;
                  actors[i]->curscale+=events[actors[i]->eventnumber]->dscale;
                  break;
               case sprite:
                  actors[i]->curx+=events[actors[i]->eventnumber]->dx;
                  actors[i]->cury+=events[actors[i]->eventnumber]->dy;
                  actors[i]->curscale+=events[actors[i]->eventnumber]->dscale;
                  break;
               default:
                  Error("UPDATE: Illegal event type\n");
                  break;
               }
            }
         }
continueloop:
      i++;
      }
}

/*
=================
=
= DrawBackground
=
=================
*/
void DrawBackground (char * name)
{
   byte * shape;
   byte * buf;
   byte * src;
   lpic_t * p;
   int i;
   int plane;


   shape=W_CacheLumpName(name,PU_CACHE);
   p=(lpic_t *)shape;

   for (plane=0;plane<4;plane++)
      {
      buf=filmbuffer;
      src=shape+8+(plane*p->height);
      VGAWRITEMAP(plane);
      for (i=plane;i<p->width;i+=4,src+=(p->height<<2),buf++)
         DrawFilmPost(buf,src,p->height);
      }
}

/*
=================
=
= DrawScrollingBackground
=
=================
*/
int DrawScrollingBackground (char * name, int x)
{
   byte * shape;
   byte * buf;
   byte * src;
   lpic_t * p;
   int i;
   int xx;
   int plane;

   shape=W_CacheLumpName(name,PU_CACHE);
   p=(lpic_t *)shape;

   if (x<0)
      x+=(p->width<<8);
   if (x>=(p->width<<8))
      x-=(p->width<<8);

   xx=x>>8;
   for (plane=0;plane<4;plane++)
      {
      buf=filmbuffer;
      src=shape+8;
      VGAWRITEMAP(plane);
      for (i=plane+xx;i<(p->width+xx);i+=4,buf++)
         {
         if (i>=p->width)
            DrawFilmPost(buf,src + ((i-p->width)*p->height),p->height);
         else
            DrawFilmPost(buf,src + (i*p->height),p->height);
         }
      }
   return x;
}


/*
=================
=
= DrawBackdrop
=
=================
*/
void DrawBackdrop (char * name)
{
   byte * shape;
   patch_t * p;
   int i;
   int toppost;
   int offset;
   int length;
   byte * src;
   byte * buf;
   int plane;

   shape=W_CacheLumpName(name,PU_CACHE);
   p=(patch_t *)shape;

   toppost=-p->topoffset;
   for (plane=0;plane<4;plane++)
      {
      buf=filmbuffer;
      VGAWRITEMAP(plane);
      for (i=plane;i<p->width;i+=4,buf++)
         {
         src=p->collumnofs[i]+shape;
         offset=*(src++);
         for (;offset!=255;)
            {
            length=*(src++);
            DrawFilmPost(buf + ylookup[toppost+offset],src,length);
            src+=length;
            offset=*(src++);
            }
         }
      }
}

/*
=================
=
= DrawScrollingBackdrop
=
=================
*/
int DrawScrollingBackdrop (char * name, int x)
{
   byte * shape;
   patch_t * p;
   int i;
   int toppost;
   int xx;
   int offset;
   int length;
   byte * src;
   byte * buf;
   int plane;

   shape=W_CacheLumpName(name,PU_CACHE);
   p=(patch_t *)shape;

   if (x<0)
      x+=(p->width<<8);
   if (x>=(p->width<<8))
      x-=(p->width<<8);

   xx=x>>8;
   toppost=-p->topoffset;
   for (plane=0;plane<4;plane++)
      {
      VGAWRITEMAP(plane);
      buf=filmbuffer;
      for (i=plane+xx;(i<p->width+xx);i+=4,buf++)
         {
         if (i>=p->width)
            src=p->collumnofs[i-p->width]+shape;
         else
            src=p->collumnofs[i]+shape;
         offset=*(src++);
         for (;offset!=255;)
            {
            length=*(src++);
            DrawFilmPost(buf + ylookup[toppost+offset],src,length);
            src+=length;
            offset=*(src++);
            }
         }
      }
   return x;
}



/*
=================
=
= ScaleFilmPost
=
=================
*/
void ScaleFilmPost (byte * src, byte * buf)
{
   int  offset;
   int  length;
   int  topscreen;
   int  bottomscreen;


   offset=*(src++);
   for (;offset!=255;)
      {
      length=*(src++);
      topscreen = sprtopoffset + (dc_invscale*offset);
      bottomscreen = topscreen + (dc_invscale*length);
      dc_yl = (topscreen+SFRACUNIT-1)>>SFRACBITS;
      dc_yh = (bottomscreen-FRACUNIT)>>SFRACBITS;
      if (dc_yh >= 200)
         dc_yh = 199;
      if (dc_yl < 0)
         dc_yl = 0;
      if (dc_yl <= dc_yh)
         {
         dc_source=src-offset;
         R_DrawFilmColumn (buf);
         }
      src+=length;
      offset=*(src++);
      }

}


/*
=================
=
= DrawSprite
=
=================
*/
void DrawSprite (char * name, int x, int y, int height, int origheight)
{
   byte *shape;
   int      frac;
   patch_t *p;
   int      x1,x2;
   int      tx;
   int      xcent;
   byte *   buf;

//   SimpleScaleShape(x,y,W_GetNumForName(name),height,origheight);
//   return;

   height>>=1;
   if (!height)
      return;
   dc_invscale = (height<<17)/origheight;
   buf=(byte *)filmbuffer;
   shape=W_CacheLumpName(name,PU_CACHE);
   p=(patch_t *)shape;
   tx=-p->leftoffset;
   xcent=(x<<SFRACBITS)-(height<<SFRACBITS)+(SFRACBITS/2);
//
// calculate edges of the shape
//
        x1 = (xcent+(tx*dc_invscale))>>SFRACBITS;
        if (x1 >= 320)
                return;               // off the right side
        tx+=p->width;
        x2 = ((xcent+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
        if (x2 < 0)
                return;         // off the left side

   dc_iscale=(origheight<<15)/height;

//
// store information in a vissprite
//
   if (x1<0)
      {
      frac=dc_iscale*(-x1);
      x1=0;
      }
   else
      frac=0;
   x2 = x2 >= 320 ? 319 : x2;

   dc_ycenter=y;
   dc_texturemid=(((origheight>>1)+p->topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
   sprtopoffset=(y<<16) - FixedMul(dc_texturemid,dc_invscale);


   for (; x1<=x2 ; x1++, frac += dc_iscale)
     {
     VGAWRITEMAP(x1&3);
     ScaleFilmPost(((p->collumnofs[frac>>SFRACBITS])+shape),buf+(x1>>2));
     }
}

/*
=================
=
= DrawPalette
=
=================
*/
void DrawPalette (char * name)
{
   byte * pal;
   byte pal2[768];

   pal=W_CacheLumpName(name,PU_CACHE);
   memcpy(&pal2[0],pal,768);
   VL_NormalizePalette(&pal2[0]);
   SwitchPalette(&pal2[0],35);
}


/*
=================
=
= DrawFadeout
=
=================
*/
void DrawFadeout ( int time )
{
   VL_FadeOut (0, 255, 0, 0, 0, time);
   CalcTics();
   CalcTics();
   VL_ClearVideo (0);
}

/*
=================
=
= DrawEvents
=
=================
*/
void DrawEvents ( void )
{
   int i;
   int done;



   // ChangePalette

   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         {
         if (events[actors[i]->eventnumber]->type==palette)
            {
            DrawPalette(&(events[actors[i]->eventnumber]->name[0]));
            FreeActor(i);
            break;
            }
         else if (events[actors[i]->eventnumber]->type==fadeout)
            {
            DrawFadeout(events[actors[i]->eventnumber]->length);
            FreeActor(i);
            break;
            }
         }

   done=0;
   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         {
         switch (events[actors[i]->eventnumber]->type)
            {
            // Draw Background
            case background:
               DrawBackground(&(events[actors[i]->eventnumber]->name[0]));
               done=1;
               break;
            // Draw Scrolling Background
            case backgroundscroll:
               actors[i]->curx=DrawScrollingBackground(&(events[actors[i]->eventnumber]->name[0]),actors[i]->curx);
               done=1;
               break;
            }
         if (done==1)
            break;
         }

   // Draw Background Sprites

   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         if (events[actors[i]->eventnumber]->type==bkgndsprite)
            {
            DrawSprite(&(events[actors[i]->eventnumber]->name[0]),
                       actors[i]->curx>>8,
                       actors[i]->cury>>8,
                       actors[i]->curscale>>8,
                       events[actors[i]->eventnumber]->origheight);
            }


   done=0;
   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         {
         switch (events[actors[i]->eventnumber]->type)
            {
            // Draw Backdrop
            case backdrop:
               DrawBackdrop(&(events[actors[i]->eventnumber]->name[0]));
               done=1;
               break;
            // Draw Scrolling Backdrop
            case backdropscroll:
               actors[i]->curx=DrawScrollingBackdrop(&(events[actors[i]->eventnumber]->name[0]),actors[i]->curx);
               done=1;
               break;
            }
         if (done==1)
            break;
         }

   // Draw foreground Sprites

   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         if (events[actors[i]->eventnumber]->type==sprite)
            {
            DrawSprite(&(events[actors[i]->eventnumber]->name[0]),
                       actors[i]->curx>>8,
                       actors[i]->cury>>8,
                       actors[i]->curscale>>8,
                       events[actors[i]->eventnumber]->origheight);
            }

}

/*
=================
=
= DeleteEvent
=
=================
*/
void DeleteEvent( eventtype type, actortype * actor )
{
   int i;

   for (i=0;i<=lastfilmactor;i++)
      if (actors[i]!=NULL)
         if ((events[actors[i]->eventnumber]->type==type) && (actors[i]!=actor))
            {
            FreeActor(i);
            if (i==lastfilmactor)
               lastfilmactor--;
            }
}


/*
=================
=
= AddEvents
=
=================
*/
void AddEvents( void )
{
   actortype * actor;

   while ((events[currentevent]->time<=filmtics) && (currentevent<eventindex))
      {
      actor=GetFreeActor();
      actor->eventnumber=currentevent;
      switch (events[actor->eventnumber]->type)
         {
         case palette:
            actor->tics=-1;
            break;
         case fadeout:
            actor->tics=-1;
            break;
         case background:
            DeleteEvent (background, actor);
            actor->tics=-1;
            break;
         case backdrop:
            DeleteEvent (backdrop, actor);
            actor->tics=-1;
            break;
         case backgroundscroll:
            DeleteEvent (backgroundscroll, actor);
            actor->curx=0;
            actor->tics=-1;
            break;
         case backdropscroll:
            DeleteEvent (backdropscroll, actor);
            actor->curx=0;
            actor->tics=-1;
            break;
         case bkgndsprite:
            actor->tics=events[actor->eventnumber]->length;
            actor->curx=events[actor->eventnumber]->x<<8;
            actor->cury=events[actor->eventnumber]->y<<8;
            actor->curscale=events[actor->eventnumber]->scale<<8;
            break;
         case sprite:
            actor->tics=events[actor->eventnumber]->length;
            actor->curx=events[actor->eventnumber]->x<<8;
            actor->cury=events[actor->eventnumber]->y<<8;
            actor->curscale=events[actor->eventnumber]->scale<<8;
            break;
         default:
            Error("ADD: Illegal event type\n");
            break;
         }
      currentevent++;
      }
}


/*
==============
=
= CacheScriptFile
=
==============
*/

void CacheScriptFile (char *filename)
{
	long            size;
   int lump;

   lump=W_GetNumForName(filename)+1;

   scriptbuffer=W_CacheLumpNum(lump,PU_CACHE);
	size = W_LumpLength(lump);

	script_p = scriptbuffer;
	scriptend_p = script_p + size;
	scriptline = 1;
	endofscript = false;
	tokenready = false;
}


/*
=================
=
= GrabMovieScript
=
=================
*/

void GrabMovieScript (char const *basename)
{
        char            script[256];

//
// read in the script file
//
        strcpy (script, basename);
        strcat (script,".ms");
//        LoadScriptFile (script);
        CacheScriptFile ((char *)basename);
        ParseMovieScript ();

}
void InitializeMovie ( void )
{
   FirstTimeInitialize();
   movielength=0;
   InitializeFilmActors();
   InitializeEvents();
}

void CleanupMovie ( void )
{
   CleanupEvents();
   CleanupActors();
}

void PlayMovie ( char * name )
{
   InitializeMovie();
   GrabMovieScript (name);
   filmbuffer=(byte *)bufferofs;
   IN_ClearKeysDown();
   for (filmtics=0;filmtics<movielength;filmtics+=dtime)
      {
      AddEvents();
      DrawEvents();
      UpdateEvents();
      FlipPage();
      CalcTics();
      dtime=tics;
      filmbuffer=(byte *)bufferofs;
      if ((LastScan) || IN_GetMouseButtons())
         break;
      }
   CleanupMovie();
}



