#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

SIGNAL(SIG_UART_RECV)
{
  Terminal(UARTGetChar());
  UARTWrite("usbn>");
}



SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}


int togl=0;


/**
 * extract command message 
 */ 

void ParseCommand(char *buf)
{
  	int i;

	UARTWrite("new command\r\n");

	switch(buf[0])
	{
		case 0x00:
			// uart
		break:


		default:
				// unkown
	}
/*
	
  USBNWrite(TXC1,FLUSH);
  for(i=0;i<64;i++)
    USBNWrite(TXD1,i);

  USBNWrite(TXC1,TX_LAST+TX_EN);
*/

}



// called at transfer irq
void TransferISR()
{
	UARTWrite("called after trans\r\n");
  /*
  //UARTWrite("ready for next\r\n");
  int i;

  USBNWrite(TXC1,FLUSH);
  
  for(i=0;i<56;i++)
    USBNWrite(TXD1,i);


  USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
  */
}


int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x0000);
  USBNDeviceBCDDevice(0x0000);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("www.ixbat.de");
  USBNDeviceProduct	("tinkerface");
  USBNDeviceSerialNumber("2006-10-05");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);


  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&ParseCommand);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,&TransferISR);

  
  USBNInitMC();

  // start usb chip
  USBNStart();
  UARTWrite("waiting for enumaration signal...\r\n");

  while(1);
}


