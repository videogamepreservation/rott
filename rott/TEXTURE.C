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

int32  texture_u;
int32  texture_v;
int32  texture_count;
int32  texture_du;
int32  texture_dv;
byte * texture_source;
byte * texture_dest;
byte * texture_destincr;

void XDominantFill(void)
   {
   int32 sx,sx1,sx2;
   byte  color;
   int32 i;
   int32 offset;
   int32 buffpos;
   int32 *buffptr;
   scan_t *scantrav;
   fixed u1,v1,u2,v2,u,v,du,dv,oneoverw1,oneoverw2,dx,startx,endx;

   _numscanlines = _maxscanline - _minscanline;
   PreprocessScanlines();

   offset = Xmax*(_minscanline-2);

   for(i = 0; i <= _numscanlines+3;offset += Xmax,i++)
      {
      if (Scanline[i] == Scanline[i]->next)
         continue;

      scantrav = Scanline[i];
      oneoverw1 = FixedDiv2(FIXED1,scantrav->next->w);
      u1 = FixedMul(scantrav->next->u,oneoverw1);
      v1 = FixedMul(scantrav->next->v,oneoverw1);

      oneoverw2 = FixedDiv2(FIXED1,scantrav->prev->w);
      u2 = FixedMul(scantrav->prev->u,oneoverw2);
      v2 = FixedMul(scantrav->prev->v,oneoverw2);

      startx = scantrav->next->value;
      endx = scantrav->prev->value;

      dx = ((endx - startx)<<16);
      if (dx)
         {
         du = FixedDiv2(u2-u1,dx);
         dv = FixedDiv2(v2-v1,dx);
         }
      else
         {
         du = dv = 0;
         }

      u = u1;
      v = v1;
      for(scantrav = Scanline[i]->next;scantrav != Scanline[i];)
         {
         texture_count = scantrav->next->value-scantrav->value;
         scantrav = scantrav->next->next;
         texture_destincr = &BufferPosition[scantrav->value];
         texture_dest = &display_buffer[offset];
         texture_source = &_texture[0];
         texture_u = u;
         texture_v = v;
         texture_du = du;
         texture_dv = dv;
         TextureLine();
         }
      }
   }

