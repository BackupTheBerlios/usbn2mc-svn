#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#include "usbn2mc.h"


SIGNAL(SIG_UART_RECV)
{
}



SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
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

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("usbserial");
  USBNDeviceSerialNumber("2007-03-02");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);//function
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);

  
  sei();
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
}


