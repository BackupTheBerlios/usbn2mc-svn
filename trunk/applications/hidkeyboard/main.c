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
  
  USBNWrite(TXC1,FLUSH);  //enable the TX (DATA1)
  USBNWrite(TXD1,0x55);
  //USBNWrite(TXC1,TX_TOGL+TX_EN+TX_LAST);  //enable the TX (DATA1)
  USBNWrite(TXC1,TX_EN+TX_LAST);  //enable the TX (DATA1)
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
char ReportDescriptorKeyboard[] = { 
	5, 1, // Usage_Page (Generic Desktop) 
	9, 6, // Usage (Keyboard) 
	0xA1, 1, // Collection (Application) 
	5, 7, // Usage page (Key Codes) 
	0x19, 224, // Usage_Minimum (224) 
	0x29, 231, // Usage_Maximum (231) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 1, // Logical_Maximum (1) 
	0x75, 1, // Report_Size (1) 
	0x95, 8, // Report_Count (8) 
	0x81, 2, // Input (Data,Var,Abs) = Modifier Byte 
	0x81, 1, // Input (Constant) = Reserved Byte 
	0x19, 0, // Usage_Minimum (0) 
	0x29, 101, // Usage_Maximum (101) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 101, // Logical_Maximum (101) 
	0x75, 8, // Report_Size (8) 
	0x95, 6, // Report_Count (6) 
	0x81, 0, // Input (Data,Array) = Keycode Bytes(6) 
	5, 8, // Usage Page (LEDs) 
	0x19, 1, // Usage_Minimum (1) 
	0x29, 5, // Usage_Maximum (5) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 1, // Logical_Maximum (1) 
	0x75, 1, // Report_Size (1) 
	0x95, 5, // Report_Count (5) 
	0x91, 2, // Output (Data,Var,Abs) = LEDs (5 bits) 
	0x95, 3, // Report_Count (3) 
	0x91, 1, // Output (Constant) = Pad (3 bits) 
	0xC0 // End_Collection 
}; 

/*************** usb requests  **************/

// reponse for requests on interface
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep)
{
  // 81 06 22 get report descriptor
  switch(req->bRequest)
  {
    case GET_DESCRIPTOR:
        ep->Index=0;
	ep->DataPid=1;
        ep->Size=59;
	ep->Buf=ReportDescriptorKeyboard;
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
    0x22,0x00,  // total length with first interface ...
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
    0x10,0x01,	// hid class spec
    0x00,	//country
    0x01,	// number of hid descriptors to flollow
    0x22,	// descriptor type
    0x3b,	// total length of report descriptor
    0x00,
    //EP1 Descriptor
    0x07,             // length of ep descriptor
    0x05,             // descriptor type= endpoint
    0x81,             // endpoint address (e.g. in ep1)
    0x03,             // transfer art ( bulk )
    0x08,0x00,  // fifo size
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






