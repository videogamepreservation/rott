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
#include "rottser.h"
#include "_rt_ser.h"
#include "rt_ser.h"
#include "rt_def.h"
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>


#include <bios.h>
//MED
#include "memcheck.h"

char	localbuffer[MAXPACKET*2+2];
char  serialpacket[MAXPACKET];
int   serialpacketlength;

int   inescape;
int   newpacket;
int   uart;
int   irq;
int   baudrate;
que_t		inque, outque;

enum {UART_8250, UART_16550} uart_type;

int			modem_status = -1;
int			line_status = -1;


void (__interrupt __far *oldirqvect) () = NULL;

int			irqintnum;


void talk (void)
{
	int c;

	printf ("Talk mode...  Press ESC to return\n\n");
	while (TRUE)
	{
		/* Check for keypress */
      if (_bios_keybrd (_KEYBRD_READY))
		{
         if ((c = _bios_keybrd (_KEYBRD_READ) & 0xff) == ESC)
			{
				/* ESC key -- Exit talk mode -- Flush both sides */
            write_byte (c);
				while (read_byte () != -1)
				;
            while (_bios_keybrd (_KEYBRD_READY))
               _bios_keybrd (_KEYBRD_READ);
				printf ("\n\n");
				return;
			}
			if (c == 0x0D)		/* Change cr to crlf */
				c = 0x0A;
         write_byte (c);
         printf ("%c\n", c);
		}
		/* Check for received byte */
		if ((c = read_byte ()) != -1)
         {
         if (c==ESC)
            {
				while (read_byte () != -1)
				;
            while (_bios_keybrd (_KEYBRD_READY))
               _bios_keybrd (_KEYBRD_READ);
				printf ("\n\n");
				return;
            }
         printf ("%c\n", c);
         }
	}
}


/*
===============
=
= SetupModemGame
=
===============
*/

void SetupModemGame ( void )
{
   serialdata_t * ser;

   ser=(serialdata_t *)&rottcom->data[0];
   irq=ser->irq;
   uart=ser->uart;
   baudrate=ser->baud;
   InitPort();
//   talk();
}

/*
===============
=
= ShutdownModemGame
=
===============
*/

void ShutdownModemGame ( void )
{
   ShutdownPort();
}


/*
===============
=
= EnablePort
=
===============
*/

void EnablePort ( void )
{
   int mcr;

//
// prepare for interrupts
//
	OUTPUT( uart + INTERRUPT_ENABLE_REGISTER, 0 );	/* Turn off interrupts */

	mcr = INPUT( uart + MODEM_CONTROL_REGISTER );	/* Get modem status */
	mcr |= MCR_OUT2;											/* Set GPO 2 */
	mcr &= ~MCR_LOOPBACK;									/* Turn off loopback test */
   mcr |= MCR_DTR;                                 /* Set DTR */
   mcr |= MCR_RTS;                                 /* Set RTS */
	OUTPUT( uart + MODEM_CONTROL_REGISTER, mcr );	/* Set modem status */

	INPUT( uart );												/* Clear Rx interrupts */
	INPUT( uart + INTERRUPT_ID_REGISTER );				/* Clear Tx interrupts */


//
// hook the irq vector
//
	irqintnum = irq + 8;

   oldirqvect = _dos_getvect(irqintnum);
   _dos_setvect (irqintnum, isr_8250);

	OUTPUT( 0x20 + 1, INPUT( 0x20 + 1 ) & ~(1<<irq) );

	CLI();

// enable RX and TX interrupts at the uart
// also enable Line Status interrupts to watch for errors.

	OUTPUT( uart + INTERRUPT_ENABLE_REGISTER,
      IER_RX_DATA_READY + IER_TX_HOLDING_REGISTER_EMPTY);

// enable interrupts through the interrupt controller

	OUTPUT( 0x20, 0xc2 );

// Set DTR

	OUTPUT( uart + MODEM_CONTROL_REGISTER,
           INPUT( uart + MODEM_CONTROL_REGISTER ) | MCR_DTR );

	STI();
}

/*
===============
=
= InitPort
=
===============
*/

void InitPort ( void )
{
	int	temp;
	unsigned long divisor;

//
// init com port settings
//

   inque.head=0;
   inque.tail=0;
   inque.size=0;
   outque.head=0;
   outque.tail=0;
   outque.size=0;

//
// check for a 16550
//
	OUTPUT( uart + FIFO_CONTROL_REGISTER, FCR_FIFO_ENABLE + FCR_TRIGGER_04 );
	temp = INPUT( uart + INTERRUPT_ID_REGISTER );
	if ( ( temp & 0xf8 ) == 0xc0 )
	{
		uart_type = UART_16550;
		printf ("UART is a 16550\n");
	}
	else
	{
		uart_type = UART_8250;
		OUTPUT( uart + FIFO_CONTROL_REGISTER, 0 );
		printf ("UART is an 8250\n");
   }

   EnablePort ();
}



/*
=============
=
= ShutdownPort
=
=============
*/

void ShutdownPort ( void )
{
	OUTPUT( uart + INTERRUPT_ENABLE_REGISTER, 0 );	/* Turn off interrupts */
//   OUTPUT( uart + MODEM_CONTROL_REGISTER, 0 );     /* Clear modem status */
//   OUTPUT( uart + FIFO_CONTROL_REGISTER, 0 );      /* Clear fifo status */

	OUTPUT( 0x20 + 1, INPUT( 0x20 + 1 ) | (1<<irq) );

   _dos_setvect (irqintnum,oldirqvect);                 /* Return to orig. inter. */

}

int read_byte( void )
{
   int   c;

   if (inque.size == 0)
      return -1;

   c = inque.data[QueSpot(inque.tail++)];
   inque.size--;

   return c;
}


void write_byte( unsigned char c )
{
   outque.data[QueSpot(outque.head++)] = c;
   outque.size++;

   if ( INPUT( uart + LINE_STATUS_REGISTER ) & 0x40)
      jump_start();
}


void write_bytes( char *buf, int count )
{
   if (QueSpot(outque.head) + count >= QUESIZE)                /* About to wrap around */
   {
      while (count--)
         write_byte (*buf++);
   }
   else
   {
      memcpy(outque.data + QueSpot(outque.head), buf, count);  /* Write all at once */
      outque.head += count;
      outque.size += count;
      if ( INPUT( uart + LINE_STATUS_REGISTER ) & 0x40)
         jump_start();
   }
}


/*
================
=
= write_buffer
=
================
*/

void write_buffer( char *buffer, unsigned int count )
{
// if this would overrun the buffer, throw everything else out
	if (outque.size + count > QUESIZE)
	{
		outque.tail = outque.head;
		outque.size = 0;
	}

   write_bytes (buffer, count);

}


/*
==============
=
= isr_8250
=
==============
*/

void interrupt isr_8250(void)
{
	int c;
	int	count;

	while (1)
	{
		switch( INPUT( uart + INTERRUPT_ID_REGISTER ) & 7 )
		{
//
// receive one byte for the UART 8250,
// as many as possible for the UART 16550
//
      case IIR_RX_DATA_READY_INTERRUPT :
         do
			{
            c = INPUT( uart + RECEIVE_BUFFER_REGISTER );
            inque.data[QueSpot(inque.head++)] = c;
            inque.size++;
			} while ( uart_type == UART_16550 && INPUT( uart + LINE_STATUS_REGISTER ) & LSR_DATA_READY );
         break;

//
// transmit one byte for the UART 8250
// 16 bytes for the UART 16550
//
      case IIR_TX_HOLDING_REGISTER_INTERRUPT :
			if (outque.size != 0)
			{
            if (uart_type == UART_16550)
					count = 16;
				else
					count = 1;
				do
				{
               c = outque.data[QueSpot(outque.tail++)];
               outque.size--;
               OUTPUT( uart + TRANSMIT_HOLDING_REGISTER, c );
				} while (--count && outque.size != 0);
			}
         break;

      case IIR_MODEM_STATUS_INTERRUPT :
			modem_status = INPUT( uart + MODEM_STATUS_REGISTER );
			break;

// not enabled
		case IIR_LINE_STATUS_INTERRUPT :
			line_status = INPUT( uart + LINE_STATUS_REGISTER );
         break;

//
// done
//

		default :
         OUTPUT( 0x20, 0x20 );
			return;
		}
	}
}


/*
===============
=
= jump_start
=
= Start up the transmition interrupts by sending the first char
===============
*/

void jump_start( void )
{
	int c;

   if (outque.size != 0)
	{
      c = outque.data [QueSpot(outque.tail++)];
      outque.size--;
      OUTPUT( uart, c );
	}
}


/*
================
=
= ReadSerialPacket
=
================
*/

boolean ReadSerialPacket (void)
{
	int	c;

// if the buffer has overflowed, throw everything out


	if (inque.size > QUESIZE - 4)	// check for buffer overflow
	{
		inque.tail = inque.head;
		inque.size = 0;
		newpacket = true;
		return false;
	}

	if (newpacket)
	{
      serialpacketlength = 0;
		newpacket = 0;
	}

	do
	{
		if ((c = read_byte ()) < 0)
			return false;      // haven't read a complete packet
		if (inescape)
		{
			inescape = false;
			if (c!=FRAMECHAR)
			{
				newpacket = 1;

				return true;    // got a good packet
			}
		}
		else if (c==FRAMECHAR)
		{
			inescape = true;
			continue;			// don't know yet if it is a terminator
		}						// or a literal FRAMECHAR

      if (serialpacketlength >= MAXPACKET)
		{
			continue;			// oversize packet
		}

      serialpacket[serialpacketlength] = c;
      serialpacketlength++;
	} while (1);

}

/*
=============
=
= WriteSerialPacket
=
=============
*/

void WriteSerialPacket (char *buffer, int len)
{
	int		b;

	b = 0;
	if (len > MAXPACKET)
	{
		return;
	}

	while (len--)
	{
		if (*buffer == FRAMECHAR)
			localbuffer[b++] = FRAMECHAR;	// escape it for literal
		localbuffer[b++] = *buffer++;
	}

	localbuffer[b++] = FRAMECHAR;
	localbuffer[b++] = 0;

	write_buffer (localbuffer, b);
}


#if 0
/*
================
=
= ReadSerialPacket
=
================
*/

boolean ReadSerialPacket ( byte * packet, short * length )
{
	int	c;

// if the buffer has overflowed, throw everything out


	if (inque.size > QUESIZE - 4)	// check for buffer overflow
	{
		inque.tail = inque.head;
		inque.size = 0;
		newpacket = true;
		return false;
	}

	if (newpacket)
	{
      (*length) = 0;
		newpacket = 0;
	}

	do
	{
		if ((c = read_byte ()) < 0)
			return false;      // haven't read a complete packet
		if (inescape)
		{
			inescape = false;
			if (c!=FRAMECHAR)
			{
				newpacket = 1;

				return true;    // got a good packet
			}
		}
		else if (c==FRAMECHAR)
		{
			inescape = true;
			continue;			// don't know yet if it is a terminator
		}						// or a literal FRAMECHAR

      if ((*length) >= MAXPACKET)
		{
			continue;			// oversize packet
		}

      packet[(*length)] = c;
      (*length)++;
	} while (1);

}
#endif

