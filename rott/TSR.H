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
#ifndef TSR_H
#define TSR_H

//---------------------------------------------------------------------------

#define TSR_VALIDATE_TIMEOUT	19		// 18.2 ticks = 1 second
#define TSR_VALIDATE_QUERY		"\r\r?\r?\r?\r\r"

#define TSR_MAX_LENGTH			80

//---------------------------------------------------------------------------

typedef struct {
	char           copyright[TSR_MAX_LENGTH];			// Pointer to driver Copyright message

	short          major;				// Driver Version Number
	short				minor;

	short				count;				// Count of available logical devices
} TsrDrvOpenPacket;



typedef struct {
	char           copyright[TSR_MAX_LENGTH];			// Pointer to device Copyright message
} TsrDevOpenPacket;



typedef struct {
	long				timestamp;
	unsigned short	period;
	unsigned short button;
	short          tx;
	short          ty;
	short          tz;
	short          rx;
	short          ry;
	short          rz;
} TsrForcePacket;



typedef struct {
	long           timestamp;
	unsigned short period;
	unsigned short button;
} TsrButtonPacket;


typedef struct {
	unsigned long data;		// Should be 0xFF0000FF
} TsrCommandPacket;

#define TSRCMD_DATA	0xFF0000FF


//---------------------------------------------------------------------------
//----------------------------- WARNING -------------------------------------
//----------------- These are private structures and should -----------------
//-------------------- be removed from the final release --------------------
//---------------------------------------------------------------------------

#ifdef PRIVATE_STRUCTS

typedef struct {
	TsrCommandPacket command;

	union {
		short vector;
		struct {
			unsigned short port;
			unsigned short irq;
			unsigned short base;
			unsigned short available;
		} info;
	} u;
} TsrPrivatePacket;

#endif

//---------------------------------------------------------------------------

typedef union {
	TsrCommandPacket	command;

	TsrDrvOpenPacket	drvOpen;
	TsrDevOpenPacket	devOpen;

	TsrForcePacket		force;
	TsrButtonPacket	button;

#ifdef PRIVATE_STRUCTS
	TsrPrivatePacket	private;
#endif
} TsrPacket;

//---------------------------------------------------------------------------
// For all TSR functions:
//	Input:
//		AX - Command Number
//		DX - Device Number
//		ES:BX - Pointer to TsrPacket (if TSR_NEED_PACKET is set)
//			Note that packet.command.data MUST be TSRCMD_DATA for the
//			call to not fail
// Output:
//		AX - Error code (0=successful)

#define TSR_NEED_PACKET			0x8000		// This function needs a valid packet

// TSR Interrupt Functions
#define TSR_DRIVER_CLOSE	 	0x0
#define TSR_DRIVER_OPEN			(TSR_NEED_PACKET | 0x1)
#define TSR_DEVICE_CLOSE		0x2
#define TSR_DEVICE_OPEN			(TSR_NEED_PACKET | 0x3)

#define TSR_DEVICE_DISABLE		0x10
#define TSR_DEVICE_ENABLE		0x11

#define TSR_DEVICE_GETFORCE	(TSR_NEED_PACKET | 0x20)
#define TSR_DEVICE_GETBUTTONS	(TSR_NEED_PACKET | 0x21)

#define TSR_DEVICE_SET_EVT_HANDLER	0x50

// Private functions (used by SpUtil)
#ifdef PRIVATE_STRUCTS

#define TSR_PRIV_CHANGE_VECTOR		0x1000	// call: bx=new vector #, return: ax=error
#define TSR_PRIV_GET_PHYS_INFO		0x1001	// call: bx=device #, return: ax=error, bx=port, cx=irq, dx=base, si=avail
#define TSR_PRIV_SET_PHYS_INFO		0x1002	// call: bx=device #, cx=port, si=irq, di=base (port=0 disables)
#define TSR_PRIV_VALIDATE_PHYS		0x1003	// return: ax=error

#endif

//---------------------------------------------------------------------------

// Errors from the device are returned in ax, ah is the major, al is the minor

#if 0
#define SI_SUCCESS					 0x0000
#define SI_GENERIC_ERROR			 0xFE00	// Generic error - no more information available
#define SI_UNKNOWN_FUNCTION		 0xFD00	// Unknown function number
#define SI_INVALID_DEVICE			 0xFC00	// Invalid device number
#define SI_CANNOT_OPEN_DEVICE		 0xFB00	// Cannot open device
#define SI_CANNOT_CLOSE_DEVICE	 0xFA00	// Cannot close device
#define SI_CANNOT_DISABLE_DEVICE	 0xF900	// Cannot disable device
#define SI_CANNOT_ENABLE_DEVICE	 0xF800	// Cannot enable device
#define SI_CANNOT_GET_TRANSLATION 0xF700	// Cannot get translation
#define SI_CANNOT_GET_ROTATION	 0xF600	// Cannot get rotation
#define SI_CANNOT_GET_BUTTON		 0xF500	// Cannot get button
#define SI_CANNOT_SET_EVT_HANDLER 0xF400	// Cannot set event handler
#define SI_CANNOT_REENTER         0xF300	// Cannot reenter TSR

// Private error codes...

#define SI_CANNOT_GET_PHYS_INFO   0x8000	// Cannot get physical device info
#define SI_CANNOT_SET_PHYS_INFO   0x8100	// Cannot get physical device info

#define SI_PHYSERR_NOT_AVAILABLE  0x00FE
#define SI_PHYSERR_BAD_DEVICE     0x00FD
#endif

//---------------------------------------------------------------------------
// Tsr Module Errors
#define SPWTSRERR_INVALID_PACKET				(SPWERR_TSR | 0x01)
#define SPWTSRERR_UNKNOWN_COMMAND			(SPWERR_TSR | 0x02)
#define SPWTSRERR_BAD_DEVICE					(SPWERR_TSR | 0x03)
#define SPWTSRERR_INFO_NOT_AVAILABLE		(SPWERR_TSR | 0x04)

//---------------------------------------------------------------------------
#endif

