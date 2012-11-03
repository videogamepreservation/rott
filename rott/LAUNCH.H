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
#define MV_Signature                 0x4d56
#define MV_SoundInt                  0x2f
#define MV_CheckForDriver            0xbc00
#define MV_GetVersion                0xbc01
#define MV_GetPointerToStateTable    0xbc02
#define MV_GetPointerToFunctionTable 0xbc03
#define MV_GetDmaIrqInt              0xbc04
#define MV_SendCommandStructure      0xbc05
#define MV_GetDriverMessage          0xbc06
#define MV_SetHotkeyScanCodes        0xbc0a
#define MV_GetPathToDriver           0xbc0b

#define OUTPUTMIXER     0x00         /* output mixer H/W select */
#define INPUTMIXER      0x40         /* input mixer select      */
#define DEFMIXER        -1           /* use last mixer selected	*/

/* left channel values */

#define L_FM            0x01
#define L_IMIXER        0x02
#define L_EXT           0x03
#define L_INT           0x04
#define L_MIC           0x05
#define L_PCM           0x06
#define L_SPEAKER       0x07
#define L_FREE          0x00
#define L_SBDAC         0x00

/* right channel values */

#define R_FM            0x08
#define R_IMIXER        0x09
#define R_EXT           0x0A
#define R_INT           0x0B
#define R_MIC           0x0C
#define R_PCM           0x0D
#define R_SPEAKER       0x0E
#define R_FREE          0x0F
#define R_SBDAC         0x0F

typedef struct
   {
   void ( far *SetMixer )( void );
   void ( far *SetVolume )( void );
   void ( far *SetFilter )( void );
   void ( far *SetCrossChannel )( void );
   void ( far *GetMixer )( void );
   void ( far *GetVolume )( void );
   void ( far *GetFilter )( void );
   void ( far *GetCrossChannel )( void );
   void ( far *ReadSound )( void );
   void ( far *FMSplit )( void );
   } MVFunc;


int PAS_CheckForDriver( void );
MVFunc far *PAS_GetFunctionTable( void );
int PAS_CallMVFunction( void ( far *function )( void ), int bx, int cx, int dx );
