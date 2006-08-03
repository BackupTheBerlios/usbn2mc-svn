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


void InterruptEndpoint(char *buf)
{
	UARTWrite("hello");
}

/* report descriptor keyboard */

char reportkeyoard[]={0x04,0x03,0x09,0x04};



/*************** usb requests  **************/

// reponse for requests on interface
void USBNInterfaceRequests(DeviceRequest *req)
{
  // 81 06 22 get report descriptor
  switch(req->bRequest)
  {
    case GET_DESCRIPTOR:
        //EP0tx.Size=4;
	//EP0tx.Buf=reportkeyboard;
    break;
    default:
      UARTWrite("unkown interface request");
   }
}



// vendor requests
void USBNDecodeVendorRequest(DeviceRequest *req)
{
UARTWrite("vendor");
#if 0
  //SendHex(req->bRequest);       // decode request code
  SendHex(req->wLength);       // decode request code
  USBNWrite(RXC0,RX_EN);
  USBNRead(RXD0);
  USBNRead(RXD0);

  //USBNWrite(TXC0,FLUSH);
  //USBNWrite(TXD0,0x24);
  //USBNWrite(TXD0,0x25);
#endif
}


// class requests
void USBNDecodeClassRequest(DeviceRequest *req)
{
  UARTWrite("class  \r\n");
  SendHex(req->bmRequestType);       // decode request code
  SendHex(req->bRequest);       // decode request code
  SendHex(req->wLength);       // decode request code

  //USBNWrite(TXC0,FLUSH);
  //USBNWrite(TXD0,0x00);
  //USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);

}




/*************** main function  **************/
int main(void)
{

  sei();

  const unsigned char easyavrDevice[] =
  { 0x12,             // 18 length of device descriptor
    0x01,       // descriptor type = device descriptor
    0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    0x00,             // device class
    0x00,             // device subclass
    0x00,       // protocol code
    0x08,       // deep of ep0 fifo in byte (e.g. 8)
    0x00,0x04,  // vendor id
    0x5D,0xC3,  // product id
    0x03,0x01,  // revision id (e.g 1.02)
    0x00,       // index of manuf. string
    0x00,             // index of product string
    0x00,             // index of ser. number
    0x01        // number of configs
  };

  // ********************************************************************
  // configuration descriptor
  // ********************************************************************

  const unsigned char easyavrConf[] =
  { 0x09,             // 9 length of this descriptor
    0x02,       // descriptor type = configuration descriptor
    0x20,0x00,  // total length with first interface ...
    0x01,             // number of interfaces
    0x01,             // number if this config. ( arg for setconfig)
    0x00,       // string index for config
    0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    0x1A,        // power for this configuration in mA (e.g. 50mA)
    //InterfaceDescriptor
    0x09,             // 9 length of this descriptor
    0x04,       // descriptor type = interface descriptor
    0x00,             // interface number
    0x00,             // alternate setting for this interface
    0x01,             // number endpoints without 0
    0x03,       	// class code
    0x01,       // sub-class code
    0x01,       // protocoll code
    0x00,       // string index for interface
    // HID Descriptor Keyboard
    0x09,	// length ot this descriptor
    0x21,	// HID Descriptortype
    0x10,0x10,	// hid class spec
    0x00,	//country
    0x01,	// number of hid descriptors to flollow
    0x22,	// descriptor type
    0x3f,	// total length of report descriptor
    //EP1 Descriptor
    0x07,             // length of ep descriptor
    0x05,             // descriptor type= endpoint
    0x81,             // endpoint address (e.g. in ep1)
    0x03,             // transfer art ( bulk )
    0x80,0x00,  // fifo size
    0x0A,             // polling intervall in ms
  };

  UARTInit();

  USBNInit(easyavrDevice,easyavrConf);

  //USBNCallbackFIFORX1(&BootLoader);

  USBNInitMC();

  // start usb chip
  USBNStart();
  
  while(1);
}






