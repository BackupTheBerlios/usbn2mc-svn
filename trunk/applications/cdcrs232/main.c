#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"


volatile int tx1togl=0; 		// inital value of togl bit

/* Device Descriptor */

const unsigned char usbrs232[] =
{ 
	0x12,             // 18 length of device descriptor
    	0x01,       // descriptor type = device descriptor
    	0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    	0x02,             // device class
    	0x00,             // device subclass
    	0x00,       // protocol code
    	0x08,       // deep of ep0 fifo in byte (e.g. 8)
    	0xc0,0x16,  // vendor id
    	0xe1,0x05,  // product id
    	0x00,0x01,  // revision id (e.g 1.02)
    	0x00,       // index of manuf. string
    	0x00,             // index of product string
    	0x00,             // index of ser. number
    	0x01        // number of configs
};

/* Configuration descriptor */

const unsigned char usbrs232Conf[] =
{ 
	0x09,             // 9 length of this descriptor
    	0x02,       // descriptor type = configuration descriptor
    	0x48,0x00,  // total length with first interface ...
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
    	0x02,       	// class code
    	0x02,       // sub-class code
    	0x01,       // protocoll code
    	0x00,       // string index for interface

    /* CDC Class-Specific descriptor */
    5,           /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    0,           /* header functional descriptor */
    0x10, 0x01,

    4,           /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    2,           /* abstract control management functional descriptor */
    0x02,        /* SET_LINE_CODING,    GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    5,           /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    6,           /* union functional descriptor */
    0,           /* CDC_COMM_INTF_ID */
    1,           /* CDC_DATA_INTF_ID */

    5,           /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    1,           /* call management functional descriptor */
    3,           /* allow management on data interface, handles call management by itself */
    1,           /* CDC_DATA_INTF_ID */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x83,        /* IN endpoint number 3 */
    0x03,        /* attrib: Interrupt endpoint */
    8, 0,        /* maximum packet size */
    100,         /* in ms */

    /* Interface Descriptor  */
    9,           /* sizeof(usbDescrInterface): length of descriptor in bytes */
    4,           /* descriptor type */
    1,           /* index of this interface */
    0,           /* alternate setting for this interface */
    2,           /* endpoints excl 0: number of endpoint descriptors to follow */
    0x0A,        /* Data Interface Class Codes */
    0,
    0,           /* Data Interface Class Protocol Codes */
    0,           /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x01,        /* OUT endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
#if UART_CFG_HAVE_USART
    6, 0,        /* maximum packet size 8->6 */
#else
    1, 0,        /* maximum packet size */
#endif
    0,           /* in ms */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x81,        /* IN endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    8, 0,        /* maximum packet size */
    0,           /* in ms */

};



/* uart interrupt (only for debugging) */

SIGNAL(SIG_UART_RECV)
{
	UARTGetChar();
	
	/*char test[]="Hallo";
	int size = 4;
	usbHIDWrite(test,size,0x05);
	*/
	
}

/* interrupt signael from usb controller */

SIGNAL(SIG_INTERRUPT0)
{
	USBNInterrupt();
}


/*************** usb class HID requests  **************/

// reponse for requests on interface
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep)
{
/*
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
*/
}



// vendor requests
void USBNDecodeVendorRequest(DeviceRequest *req)
{
}


// class requests
void USBNDecodeClassRequest(DeviceRequest *req)
{
}




/* function for sending strings over usb hid device 
 * please use max size of 64 in this version
 */

void usbHIDWrite(char hex)
{
  	int i;

  	USBNWrite(TXC1,FLUSH);  //enable the TX (DATA1)

  	USBNWrite(TXD1,0x00);	// send chars 		bei shift = 02
  	USBNWrite(TXD1,0x00);	
		USBNWrite(TXD1,hex);	
  	USBNWrite(TXD1,0x00);	

  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	

  	/* control togl bit of EP1 */
  	if(tx1togl)
  	{
  		USBNWrite(TXC1,TX_TOGL+TX_EN+TX_LAST);  //enable the TX (DATA1)
		tx1togl=0;
  	}
  	else
  	{
  		USBNWrite(TXC1,TX_EN+TX_LAST);  //enable the TX (DATA1)
		tx1togl=1;
  	}
}



/*************** main function  **************/

int main(void)
{

  	sei();			// activate global interrupts
  	UARTInit();		// only for debugging

  	// setup usbstack with your descriptors
  	USBNInit(usbrs232,usbrs232Conf);


  	USBNInitMC();		// start usb controller
  	USBNStart();		// start device stack


	
	/* stupid wait loop */
	int i,j;		 
	char key;
	char test[2];
  	while(1)
	{
		
		//key = atkeyb_getchar();
		//usbHIDWrite(key-93);
		//usbHIDWrite(key-93);
		//SendHex(key);

		//test[0]=key;
		//test[1]=0x00;
		//UARTWrite(test);
		



	    for(j=0;j<0xFFFF;j++){}
		usbHIDWrite(0x00);
/*	
	  int size = 4;
	  
	  usbHIDWrite(test,size,0x04);
	  */
	}
}






