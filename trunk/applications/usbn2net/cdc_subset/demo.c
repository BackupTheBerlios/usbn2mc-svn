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

#include "uip/uip/uip.h"
#include "uip/uip/uip_arp.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

// log function for tcp ip stack
void uip_log(char *m);

// get data from usb bus
void usbn2net_reveive(char* data);

// send ethernet frames to host
void usbn2net_send();
//void usbn2net_send(char* data, int len);

//proceed ethernet frame
void usbn2net_event(void);

void usbn2net_toggle();

//setup tcp ip stack
void usbn2net_init(void);


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
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&usbn2net_reveive);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0);
 
  /********************************************
   * setup uIP Stack 
   *******************************************/ 


  usbn2net_init();

 /********************************************
   * start engine 
   *******************************************/ 
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
  //libuip_loop();
}


/*
// add own vendor requests

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






/*-----------------------------------------------------------------------------------*/


int ip_incomplete=0;
int ip_length=0;

// get data from usb bus
void usbn2net_reveive(char* data)
{
  //UARTWrite("isr start\r\n");
  int i;
  for(i=0;i<64;i++)
  {
    //SendHex(data[i]); 
    uip_buf[i+uip_len] = data[i];
  }
  //UARTWrite("\n\r\n\r");
 
  if(ip_incomplete)
  {

    //uip_len = ip_length-64 if result smaller than 64 else reveice next package
    if( (ip_length-64) <= 64)
    {
      ip_incomplete=0;
      // message complete
      uip_len = ip_length+14;
    /*  
      UARTWrite("\n\r\n\r");
      UARTWrite("ip complete ");
      SendHex(uip_len);

      UARTWrite("\r\n\r\n");
      for(i=0;i<uip_len;i++)
	SendHex(uip_buf[i]);
      
      UARTWrite("\r\n\r\n");
      */
      usbn2net_event();
    }
    else
    {
      UARTWrite("ip incomplete\r\n");
    }

  }
  
  if(BUF->type == htons(UIP_ETHTYPE_IP)) 
  {
    //look into ip length field to get all data from usb bus (ip-length + 14 bytes)
    //position 0x10 and 0x11  is length	  
    ip_length= uip_buf[0x10]*0xff + uip_buf[0x11];

    // the ip packet is smaller than 64 Bytes
    if(ip_length <=50)
    {
      uip_len = ip_length+14;
      // start uIP Event
      usbn2net_event();
    }
    else
    {
      // paket is not complete!
      ip_incomplete=1;
      uip_len=64;
    }
  }
 
  
  // only allow this if if no further data from ip frame will be received
  // if frame is reveived complete call usbn2net_event
  if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
    uip_len=42;
    usbn2net_event();
    
    // send till uip_buf is clear 
    while(uip_len!=0)
    {
      usbn2net_event();
    }
  }

 
  
 // UARTWrite("isr stop\r\n");
}

int togl=0;
int send_blocks=0;

// send ethernet frames to host
//void usbn2net_send(char* data, int len)
void usbn2net_send()
{
  int i;
  cli();
 
  USBNWrite(TXC1,FLUSH);

  UARTWrite("**************send to bulk ");
  SendHex(uip_len);
  UARTWrite("\r\n");

  // send till len = 0 

  if(uip_len > 64)
  {
    for(i=0;i<63;i++)
      USBNWrite(TXD1,uip_buf[i+(send_blocks*64)]);

    usbn2net_toggle();
    send_blocks++;
    uip_len=uip_len-63;
    //usbn2net_send();
  }
  else {
    for(i=0;i<uip_len;i++)
      USBNWrite(TXD1,uip_buf[i]);

    usbn2net_toggle();
    uip_len=0;
  }
 
    sei();
}

void usbn2net_toggle()
{
  if(togl==0)
  {
    USBNWrite(TXC1,TX_LAST+TX_EN);
    togl=1;
  } else 
  {
    USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
    togl=0;
  }
  
}


void usbn2net_init(void)
{
  /* Initialize the device driver. */ 
  //tapdev_init();

  /* Initialize the uIP TCP/IP stack. */
  uip_init();
  uip_log("started");


  /* Initialize the HTTP server. */
  example_init();
  
  uip_arp_init();
}



void usbn2net_event(void)
{
  u8_t i, arptimer;
 // arptimer=0;

 // while(1) {
  //  cli();
    /* Let the tapdev network device driver read an entire IP packet
       into the uip_buf. If it must wait for more than 0.5 seconds, it
       will return with the return value 0. If so, we know that it is
       time to call upon the uip_periodic(). Otherwise, the tapdev has
       received an IP packet that is to be processed by uIP. */
    //uip_len = tapdev_read();
    if(uip_len == 0) {
      for(i = 0; i < UIP_CONNS; i++) {
	uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_log("len > 0");  
	  uip_arp_out();
	  //usbn2net_send();
	}
      }
      /* Call the ARP timer function every 10 seconds. */
     /* 
      if(++arptimer == 20) {	
	uip_arp_timer();
	arptimer = 0;
      }
      */
      
    } else {

      if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	uip_log("eth in event found");
	uip_arp_ipin(); //check an update arp cache
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  //tapdev_send();
	  usbn2net_send();
	  //usbn2net_send(uip_buf,uip_len);
	  /*
	  UARTWrite("Send Data ");
	  SendHex(uip_len);
	  UARTWrite("\r\n");


	  for(i=0;i<uip_len;i++)
		  SendHex(uip_buf[i]);
	  */
	}
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_log("arp in event found");
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */	
	if(uip_len > 0) {	
	  //tapdev_send();
	  //SendHex(uip_len+30);
	  //usbn2net_send(uip_buf,42);
	  usbn2net_send();
	}
      }
    }
 //    sei();
 // }
}


void uip_log(char *m)
{
  UARTWrite("uIP log message: ");
  UARTWrite(m);
  UARTWrite("\r\n");
}
