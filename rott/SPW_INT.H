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
#ifndef SPW_INT_H
#define SPW_INT_H

//==================================================================
/*
(c)1994 SpaceTec IMC Corporation.  All rights reserved.  This computer
software, including source code and object code, constitutes the
proprietary and confidential information of SpaceTec IMC Corporation
and is provided pursuant to a license agreement.  This computer software
is protected by international, federal and state law, includeing United
States Copyright Law and international treaty provisions.  Except as
expressly authorized by the license agreement, no part of this program
may be reproduced or transmitted in any form or by any means, electronic
or mechanical, modifeid, decompiled, disassembled, reverse engineered,
sold, transferred, rented or utilized for any unauthorized purpose without
the express written permission of SpaceTec IMC Corporation.
*/
//==================================================================

typedef struct Spw_IntPacket {
  unsigned char    command;	
  unsigned char    res1;		
  short            comspec;	
  long             period;	
  unsigned short   button;	
  unsigned short   buttondown;	
  short            tx;		
  short            ty;		
  short            tz;		
  short            rx;		
  short            ry;		
  short            rz;		
  short            res2;		
  short            res3;		
  unsigned short  checksum;
} Spw_IntPacket;

#define SP_BTN_1 2
#define SP_BTN_2 1
#define SP_BTN_3 4
#define SP_BTN_4 8
#define SP_BTN_5 16
#define SP_BTN_P 32

#define MENU_AMT 0x855

short SP_Open(void);
void  SP_Get(Spw_IntPacket* sp);
void  SP_Get_Btn(Spw_IntPacket* sp);
void  SP_Close(void);

//==================================================================
#endif /* SPW_INT_H */
