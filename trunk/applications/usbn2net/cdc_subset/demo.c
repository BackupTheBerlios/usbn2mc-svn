#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "uart.h"
#include "../../../firmware/usbn960x/usbnapi.h"
#include "usbn2mc.h"

#include "uip/libuip.h"

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


// this function is called by the usb core when
// the host sends data

void RX(char* data)
{
  //call uip receive engine
  //void libuip_init(void);

  libuip_reveicepacketdata(data,46);

  //void libuip_send();

}


// this function is for the uip stack to send data to the host

void TX(char* data, int len)
{
  int i;
  
  for(i=0;i<len;i++)
    USBNWrite(TXD1,data[i]);


  USBNWrite(TXC1,TX_LAST+TX_EN);
}



int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  /********************************************
   * setup USB System 
   *******************************************/

  USBNDeviceVendorID(0x0525);
  //USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x2888);
  //USBNDeviceProductID(0x9876);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("usbn2net");
  USBNDeviceSerialNumber("2006-04-12");

  USBNDeviceClass(0x02); // ethernet
  USBNDeviceSubClass(0x00);
  USBNDeviceProtocol(0x00);
  
  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&RX);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0);
 
  /********************************************
   * setup uIP Stack 
   *******************************************/ 


  libuip_init();

 /********************************************
   * start engine 
   *******************************************/ 
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  //while(1);
  libuip_loop();
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

