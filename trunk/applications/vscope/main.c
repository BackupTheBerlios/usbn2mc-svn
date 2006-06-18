#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

#include "fifo.h"

#include "vscope.h"


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
  // activate signal for next measure
  fifo_put (&vscope.fifo, PINB);

if(togl==1)
{
PORTA = 0xFF;
togl=0;
}
else {
PORTA = 0x00;
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
  //USBNAddInEndpoint(conf,interf,2,0x03,BULK,64,0,&VScopePingPongTX2); // scope data
  USBNAddInEndpoint(conf,interf,3,0x04,BULK,64,0,NULL); //result logging

  
  USBNInitMC();

  // init fifo
  //fifo_init(&vscope.fifo, fifobuffer, 1000);   


  //setup vscope state and mode
  vscope.state=STATE_DONOTHING;
  vscope.mode=MODE_NONE;
  vscope.samplerate=SAMPLERATE_1MS;

  // start usb chip
  USBNStart();

  //DDRB=0xff;
  DDRA=0xff;
  DDRB=0x00; //in port
  PORTB = 0xff; //internal pull up resistors

  int datatogl=0;

  int fifostate=1;
  while(1){
  
    if(vscope.fifo.count>=1000)
     {
	     UARTWrite("f");
     }
  int i,j;

  if(vscope.fifo.count>0 &&vscope.tx==1)
  {
    vscope.tx=0;
    USBNWrite(TXC1,FLUSH);

    USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));

    if(vscope.fifo.count<63)
      j=vscope.fifo.count;
    else j=63;
    for(i=0;i<j;i++)
      USBNBurstWrite(fifo_get_nowait(&vscope.fifo));

  
    if(datatogl==1)
    {
      USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
      datatogl=0;
    }else
    {
      USBNWrite(TXC1,TX_LAST+TX_EN);
      datatogl=1;
    }
  }
  }
}

