/**
 * Copyright 2006 Benedikt Sauter <sauter@ixbat.de>
 */
#include "uart.h"
#include "vport-uart.h"

void vport_uart_command(char *buf)
{
	// command
	switch(buf[3])
	{
		// config
		case 0x00:

		break;

		// send data over uart
		case 0x01:
			vport_uart_write(buf[2],&buf[4],buf[1]-4);
		break;

		// send data from here to usb
		case 0x02:

		break;
		default:
			asm("nop");
	}

		

}



/**
 * setup the interfaces
 */

void vport_uart_config(int port, int speed, int flags)
{

		

}

/**
 * send data from usb to rs232
 */
void vport_uart_write(int port, char * buf, int len)
{
	int i;
	for(i=0;i<len;i++)
		UARTPutChar(buf[i]);

}



/**
 * write data from intern buffer to endpoint
 */

void vport_uart_read(int port)
{

}
