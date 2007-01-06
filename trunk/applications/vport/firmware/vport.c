#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "vprotocol.h"


#include "vport-uart.h"

void Terminal(char cmd);

volatile int togl=0;

SIGNAL(SIG_UART_RECV)
{
  //UARTWrite("rec");
  
  USBNWrite(TXC1,FLUSH);
  
  USBNWrite(TXD1,UARTGetChar());

  if(togl==0){
	USBNWrite(TXC1,TX_LAST+TX_EN);
	togl=1;
    }
  else {
    USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
    togl=0;
  }
}


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}


/**
 * extract command message 
 */ 

void ParseCommand(char *buf)
{
    int i;

    //UARTWrite("new command\r\n");

    switch(buf[0])
    {
	case PART_SERIAL:
	    // uart
	    vport_uart_command(buf);
	break;

	case PART_IOPORT:

	break;
	
	default:
	    // unkown
	    asm("nop");
	}
}



// called at transfer irq
void TransferISR()
{
  //UARTWrite("called after trans\r\n");
  /*
  //UARTWrite("ready for next\r\n");
  int i;

  USBNWrite(TXC1,FLUSH);
  
  for(i=0;i<56;i++)
    USBNWrite(TXD1,i);


  USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
  */
}




int vport_msg(char *msg, int len)
{

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

  
  USBNDeviceManufacture ("FH Augsburg");
  USBNDeviceProduct	("vScope");
  USBNDeviceSerialNumber("www.vscope.de");

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

  while(1)
  {
	/* send new messages if some available */
  }
}


