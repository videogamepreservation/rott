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

extern unsigned long numBreak;
extern unsigned long numFramingError;
extern unsigned long numParityError;
extern unsigned long numOverrunError;
extern unsigned long numTxInterrupts;
extern unsigned long numRxInterrupts;

extern unsigned long writeBufferOverruns;

void reset_counters (void);
void stats (void);
int Connect ( void );
void NetISR (void);
void WritePacket (char *buffer, int len);
boolean ReadPacket (void);

void StartTime( void );
void EndTime( void );
