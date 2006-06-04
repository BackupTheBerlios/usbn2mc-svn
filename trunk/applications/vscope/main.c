#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "vscope.h"

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


SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  //UARTWrite("timer\r\n");

  if(togl==1)
  {
    PORTB = 0xFF;
    togl=0;
  }
  else {
    PORTB = 0x00;
    togl=1;
  }

}

int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x9876);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("www.vscope.de");
  USBNDeviceProduct	("VScope Device");
  //USBNDeviceSerialNumber("2006-04-24");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&VScopeCommand); // scope commands
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL); // scope data
  USBNAddInEndpoint(conf,interf,1,0x04,BULK,64,0,NULL); //result logging

  
  USBNInitMC();

  // start usb chip
  USBNStart();

  DDRB=0xff;

  while(1);
    //if(bufindex > 64)
      //bufindex=bufindex-64;
      //VscopeSendData(buffer);
    //}
}

