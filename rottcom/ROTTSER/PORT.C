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

// port.c

#include "global.h"
#include "port.h"
#include "serial.h"
#include "sercom.h"
#include "sersetup.h"
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>


union	   REGS	regs;
struct	SREGS	sregs;

que_t		inque, outque;

int irq = -1;
int uart = -1;
int comport = -1;
long baudrate = 9600;

enum {UART_8250, UART_16550} uart_type;

int			modem_status = -1;
int			line_status = -1;

void interrupt (far *oldirqvect) (void);

int			irqintnum;

/*
===============
=
= Is8250
=
===============
*/

boolean Is8250 ( void )
{
   return (uart_type == UART_8250);
}

/*
==============
=
= GetUart
=
==============
*/

void GetUart ( void )
{
	char   far *system_data;
	static int ISA_uarts[] = {0x3f8,0x2f8,0x3e8,0x2e8};
	static int ISA_IRQs[] = {4,3,4,3};
	static int MCA_uarts[] = {0x03f8,0x02f8,0x3220,0x3228};
	static int MCA_IRQs[] = {4,3,3,3};

	regs.h.ah = 0xc0;
	int86x( 0x15, &regs, &regs, &sregs );
	if ( regs.x.cflag )
	{
		if (irq == -1)
			irq = ISA_IRQs[ comport-1 ];
		if (uart == -1)
			uart = ISA_uarts[ comport-1 ];
		return;
	}
	system_data = ( char far *) ( ( (long) sregs.es << 16 ) + regs.x.bx );
	if ( system_data[ 5 ] & 0x02 )
	{
		if (irq == -1)
			irq = MCA_IRQs[ comport-1 ];
		if (uart == -1)
			uart = MCA_uarts[ comport-1 ];
	}
	else
	{
		if (irq == -1)
			irq = ISA_IRQs[ comport-1 ];
		if (uart == -1)
			uart = ISA_uarts[ comport-1 ];
	}

	printf ("Looking for UART at port 0x%x, irq %d...\n",uart,irq);
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
	int mcr;
	int	temp;
	unsigned long divisor;

//
// init com port settings
//

/*******
	regs.x.ax = 0xf3;		//f3= 9600 n 8 1
	regs.x.dx = comport - 1;
	int86 (0x14, &regs, &regs);
*******/

   inque.head=0;
   inque.tail=0;
   inque.size=0;
   outque.head=0;
   outque.tail=0;
   outque.size=0;

	printf ("Port Speed = %ld\n",baudrate);

   divisor = baudrate;

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
      if (divisor>57600L)
         {
         divisor=57600L;
         printf("WARNING: The 8250 cannot handle port speeds above 57,600\n"
                "         Setting port speed to 57,600.\n");
         }
//      printf("\nSorry, Rise of the Triad currently does not support 8250 UARTS.\n");
//      printf("This will be corrected in version 1.1.\n");
//      ShutDown();
//      exit(0);
	}

//	if ((divisor = baudrate) == 14400)
//		divisor = 19200;

	divisor = CLOCK_FREQUENCY / (16 * divisor);					/* Calc. divisor */
	OUTPUT( uart + LINE_CONTROL_REGISTER, LCR_DLAB );			/* Enable divisor */
	OUTPUT( uart + DIVISOR_LATCH_HIGH, 0 );						/* Set divisor */
	OUTPUT( uart + DIVISOR_LATCH_LOW, (unsigned char) divisor);
	OUTPUT( uart + LINE_CONTROL_REGISTER, 3 );					/* Set 8,n,1 */


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

	oldirqvect = getvect (irqintnum);

   setvect (irqintnum,isr_8250);
#if 0
   if( uart_type == UART_8250 )        /* Use different interrupt routines */
		setvect (irqintnum, isr_8250);
	else
		setvect (irqintnum, isr_16550);
#endif
	OUTPUT( 0x20 + 1, INPUT( 0x20 + 1 ) & ~(1<<irq) );

	CLI();

// enable RX and TX interrupts at the uart
// also enable Line Status interrupts to watch for errors.

	OUTPUT( uart + INTERRUPT_ENABLE_REGISTER,
      IER_RX_DATA_READY + IER_TX_HOLDING_REGISTER_EMPTY);// + IER_LINE_STATUS );

// enable interrupts through the interrupt controller

	OUTPUT( 0x20, 0xc2 );

// Set DTR

	OUTPUT( uart + MODEM_CONTROL_REGISTER,
           INPUT( uart + MODEM_CONTROL_REGISTER ) | MCR_DTR );

	STI();
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
	OUTPUT( uart + MODEM_CONTROL_REGISTER, 0 );		/* Clear modem status */
	OUTPUT( uart + FIFO_CONTROL_REGISTER, 0 );		/* Clear fifo status */

	OUTPUT( 0x20 + 1, INPUT( 0x20 + 1 ) | (1<<irq) );

	setvect (irqintnum,oldirqvect);						/* Return to orig. inter. */

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
		++writeBufferOverruns;
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

   OUTPUT( 0x20, 0x20 );

	while (1)
	{
		switch( INPUT( uart + INTERRUPT_ID_REGISTER ) & 7 )
		{
//
// receive one byte for the UART 8250,
// as many as possible for the UART 16550
//
      case IIR_RX_DATA_READY_INTERRUPT :
         ++numRxInterrupts;
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
         ++numTxInterrupts;
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
         if ( line_status & LSR_OVERRUN_ERROR )
				++numOverrunError;

			if ( line_status & LSR_PARITY_ERROR )
				++numParityError;

			if ( line_status & LSR_FRAMING_ERROR )
				++numFramingError;

			if ( line_status & LSR_BREAK_DETECT )
				++numBreak;

         break;

//
// done
//

		default :
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






















#if 0
//==========================================================================
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
// receive exactly one byte, since this is a UART 8250
//
		case IIR_RX_DATA_READY_INTERRUPT :
//I_ColorBlack (0,63,0);
			++numRxInterrupts;
			c = INPUT( uart + RECEIVE_BUFFER_REGISTER );
         inque.data[QueSpot(inque.head++)] = c;
			inque.size++;
//         if (inque.head >= QUESIZE)
//            inque.head = 0;      // wrap around
			break;

//
// transmit exactly one byte, since this is a UART 8250
//
		case IIR_TX_HOLDING_REGISTER_INTERRUPT :
//I_ColorBlack (63,0,0);
			++numTxInterrupts;
			if (outque.size != 0)
			{
            c = outque.data[QueSpot(outque.tail++)];
				outque.size--;
//            if (outque.tail >= QUESIZE)
//               outque.tail = 0;      // wrap around
				OUTPUT( uart + TRANSMIT_HOLDING_REGISTER, c );
			}
			break;

#if 0		// not enabled
*		case IIR_MODEM_STATUS_INTERRUPT :
*			modem_status = INPUT( uart + MODEM_STATUS_REGISTER );
*			break;
#endif

//
// line status
//
		case IIR_LINE_STATUS_INTERRUPT :
			line_status = INPUT( uart + LINE_STATUS_REGISTER );

			if ( line_status & LSR_OVERRUN_ERROR )
				++numOverrunError;

			if ( line_status & LSR_PARITY_ERROR )
				++numParityError;

			if ( line_status & LSR_FRAMING_ERROR )
				++numFramingError;

			if ( line_status & LSR_BREAK_DETECT )
				++numBreak;

			break;

//
// done
//
		default :
//I_ColorBlack (0,0,0);
			OUTPUT( 0x20, 0x20 );
			return;
		}
	}
}

//==========================================================================
/*
==============
=
= isr_16550
=
==============
*/

void interrupt isr_16550(void)
{
	int c;
	int	count;

	while (1)
	{
		switch( INPUT( uart + INTERRUPT_ID_REGISTER ) & 7 )
		{
//
// receive
//
		case IIR_RX_DATA_READY_INTERRUPT :
//I_ColorBlack (0,63,0);
			++numRxInterrupts;
			do
			{
				c = INPUT( uart + RECEIVE_BUFFER_REGISTER );
            inque.data[QueSpot(inque.head++)] = c;
            inque.size++;
//            if (inque.head >= QUESIZE)
//               inque.head = 0;      // wrap around
			} while ( INPUT( uart + LINE_STATUS_REGISTER ) & LSR_DATA_READY );

			break;

//
// transmit
//
		case IIR_TX_HOLDING_REGISTER_INTERRUPT :
//I_ColorBlack (63,0,0);
			++numTxInterrupts;
			if (outque.size != 0)
			{
				count = 16;
				do
				{
               c = outque.data[QueSpot(outque.tail++)];
               outque.size--;
//               if (outque.tail >= QUESIZE)
//                  outque.tail = 0;      // wrap around

					OUTPUT( uart + TRANSMIT_HOLDING_REGISTER, c );
				} while (--count && outque.size != 0);
			}
			break;

#if 0		// not enabled
*		case IIR_MODEM_STATUS_INTERRUPT :
*			modem_status = INPUT( uart + MODEM_STATUS_REGISTER );
*			break;
#endif

//
// line status
//
		case IIR_LINE_STATUS_INTERRUPT :
			line_status = INPUT( uart + LINE_STATUS_REGISTER );

			if ( line_status & LSR_OVERRUN_ERROR )
				++numOverrunError;

			if ( line_status & LSR_PARITY_ERROR )
				++numParityError;

			if ( line_status & LSR_FRAMING_ERROR )
				++numFramingError;

			if ( line_status & LSR_BREAK_DETECT )
				++numBreak;

			break;

//
// done
//
		default :
//I_ColorBlack (0,0,0);
			OUTPUT( 0x20, 0x20 );
			return;
		}
	}
}

#endif

