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

int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0603);
  USBNDeviceProductID(0x00f2);
  //USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceProduct	("USB Keyboard");
  //USBNDeviceManufacture ("Benedikt Sauter");


  //USBNDeviceClass(0x03); 
  //USBNDeviceSubClass(0x01);
  //USBNDeviceProtocol(0x01);



  
  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterfaceClass(conf,0,0x03,0x01,0x01);
  USBNAlternateSetting(conf,interf,0);

  USBNInterfaceName(conf,interf,"usbhid");
  

  USBNAddInEndpoint(conf,interf,1,0x01,INTR,64,0,NULL); // scope data

  
  USBNInitMC();
  // start usb chip
  USBNStart();

  while(1);
}






