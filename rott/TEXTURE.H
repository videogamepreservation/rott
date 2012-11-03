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
#ifndef _texture_public
#define _texture_public

extern int32  texture_u;
extern int32  texture_v;
extern int32  texture_count;
extern int32  texture_du;
extern int32  texture_dv;
extern byte * texture_source;
extern byte * texture_dest;
extern byte * texture_destincr;

void TextureLine( void );

#endif
