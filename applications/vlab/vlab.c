#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
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



// testfunction where called when data on ep2, buf is a ptr to a 64 byte field 
void SetGetPort(char *buf)
{
  #define SETPORT 0x01
  #define GETPORT 0x02
  char port,pin,value,portorg;

  port = buf[1];
  pin  = buf[2];
  value= buf[3];

  // this function is called when data where ready
  if(buf[0]==SETPORT)
  {
    if(port==1)
      portorg = PORTA;

    if(port==2)
      portorg = PORTB;
      
    pin--;
    
    ((value) ? ((portorg) |= (1<<(pin))) : ((portorg) &= ~(1<<(pin))));
  
    if(port==1)
      PORTA=portorg;

    if(port==2)
      PORTB=portorg;
  }
  else if (buf[0]==GETPORT)
  {
  }
 


  // send new port state
  USBNWrite(TXC1,FLUSH);
  if(buf[1]==1)
    USBNWrite(TXD1,PORTA);
  if(buf[1]==2)
    USBNWrite(TXD1,PORTB);

  USBNWrite(TXC1,TX_LAST+TX_EN);
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

  
  USBNDeviceManufacture ("vScopeTeam");
  USBNDeviceProduct	("vScopeDevice");
  USBNDeviceSerialNumber("2006-04-05");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,&SetGetPort);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64);

/*
  conf = USBNAddConfiguration();
 
  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,1);
  
  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64);
  USBNAddInEndpoint(conf,interf,2,0x03,BULK,8);
  USBNAddInEndpoint(conf,interf,3,0x04,BULK,64);
*/
  
  MCUCR |=  (1 << ISC01); // fallende flanke

  GICR |= (1 << INT0);
  sei();
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  DDRB=0xff;
  PORTB=0x00;
  while(1);
}


