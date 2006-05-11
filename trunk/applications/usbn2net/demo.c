#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "uart.h"
#include "../../firmware/usbn960x/usbnapi.h"
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

void wait_ms(int ms)
{
  int i;
  for(i=0;i<ms;i++)
   _delay_ms(1);
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
  USBNDeviceProduct	("usbn2net");
  USBNDeviceSerialNumber("2006-04-12");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  //USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&At89Flash);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0);

  
  MCUCR |=  (1 << ISC01); // fallende flanke

  GICR |= (1 << INT0);
  sei();
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
}


/*

//*******************************************************************
// add own vendor requests
// ********************************************************************
// decode your own vendor requests

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  //SendHex(req->bRequest);       // decode request code
  SendHex(req->wLength);       // decode request code
  USBNWrite(RXC0,RX_EN);
  USBNRead(RXD0);
  USBNRead(RXD0);

  //USBNWrite(TXC0,FLUSH);
  //USBNWrite(TXD0,0x24);
  //USBNWrite(TXD0,0x25);
}


void USBNDecodeClassRequest(DeviceRequest *req)
{
  //SendHex(req->bRequest);       // decode request code
  SendHex(req->wLength);       // decode request code
  USBNWrite(RXC0,RX_EN);
  USBNRead(RXD0);
  USBNRead(RXD0);
}
*/

