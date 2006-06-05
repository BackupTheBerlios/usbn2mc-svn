#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

#include "vscope.h"
vscope_t vscope;



#include "fifo.h"
uint8_t buffer[100];
fifo_t fifo;



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
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,&VScopePingPongTX1); // scope data
  USBNAddInEndpoint(conf,interf,2,0x03,BULK,64,0,&VScopePingPongTX2); // scope data
  USBNAddInEndpoint(conf,interf,3,0x04,BULK,64,0,NULL); //result logging

  
  USBNInitMC();

  // init fifo
  fifo_init(&fifo, buffer, 100);   


  //setup vscope state and mode
  vscope.state=STATE_DONOTHING;
  vscope.mode=MODE_NONE;

  // start usb chip
  USBNStart();

  DDRB=0xff;

  while(1)
  {
    if(fifo.count > 0)
    {
      // send scope data in ping pong mode
      VScopeSendScopeData();
    }
  }
}

