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
void usbn2net_txcallback();

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
/*
void wait_ms(int ms)
{
  int i;
  for(i=0;i<ms;i++)
   _delay_ms(1);
}

*/

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
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,&usbn2net_txcallback);
 
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
}


/*-----------------------------------------------------------------------------------*/


int in_ip_incomplete=0;
int ip_length=0;
int fillindex=0;

// get data from usb bus
void usbn2net_reveive(char* data)
{
  uip_log("receive");
  int i;
  for(i=0;i<64;i++)
  {
    uip_buf[i+fillindex] = data[i];
  }

/******************
  if(in_ip_incomplete)
  {

    //uip_len = ip_length-64 if result smaller than 64 else reveice next package
    if( (ip_length-64) <= 64)
    {
      in_ip_incomplete=0;
      // message complete
      uip_len = ip_length+14;
      usbn2net_event();
    }
    else
    {
      UARTWrite("ip incomplete\r\n");
    }
  }
  
*****************************/ 
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
      in_ip_incomplete=1;
      fillindex=fillindex+64;
      if(fillindex>=ip_length)
      {
	uip_len=ip_length;
	fillindex=0;
	usbn2net_event();
      }
    }
  }
  
  // only allow this if if no further data from ip frame will be received
  // if frame is reveived complete call usbn2net_event
  if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
    uip_len=42;
    usbn2net_event();
  
    /*
    // send till uip_buf is clear 
    while(uip_len!=0)
    {
      usbn2net_event();
    }
    */
  }
}

int togl=0;
int send_blocks=0;

// send ethernet frames to host
//void usbn2net_send(char* data, int len)
void usbn2net_send()
{
  uip_log("send");
  int i;

  // send till len = 0 

  //togl=0;

  USBNWrite(TXC1,FLUSH);
  if(uip_len > 64)
  {
    uip_log("part of message\r\n");

    for(i=0;i<64;i++)
      USBNWrite(TXD1,uip_buf[i+(send_blocks*64)]);

    send_blocks++;
    uip_len=uip_len-64;
    
    togl=1; // first icmp frame
    usbn2net_toggle();
  }
  //send complete message at once
  else {
    uip_log("short message\r\n");
    for(i=0;i<uip_len;i++)
      USBNWrite(TXD1,uip_buf[i]);

    uip_len=0;
    //usbn2net_toggle();
    USBNWrite(TXC1,TX_LAST+TX_EN);
  }
}



// is called after tx event
void usbn2net_txcallback()
{
  int i;

  if(uip_len!=0){
    UARTWrite("callback\r\n");
    USBNWrite(TXC1,FLUSH);
    for(i=0;i<34;i++)
      USBNWrite(TXD1,uip_buf[i+(send_blocks*64)]);
/*
    USBNWrite(TXD1,0x55);
    USBNWrite(TXD1,0x55);
    USBNWrite(TXD1,0x55);
    USBNWrite(TXD1,0x55);
 */ 
    uip_len=0;
    send_blocks=0;
    usbn2net_toggle();
    //USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
  }
}


// togl pid for in endpoint
void usbn2net_toggle()
{
  if(togl==0)
  {
    togl=1;
    USBNWrite(TXC1,TX_LAST+TX_EN);
  } else 
  {
    togl=0;
    USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
  }
}


void usbn2net_init(void)
{
  /* Initialize the uIP TCP/IP stack. */
  uip_init();
  
  uip_log("started");

  /* Initialize the HTTP server. */
  example_init();
  
  uip_arp_init();
}



// is only allowed to call when a complete package is in uip_buf
void usbn2net_event(void)
{
  u8_t i;
 uip_log("event"); 
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
	  usbn2net_send();
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
	  usbn2net_send();
	}
      }
    }
}


void uip_log(char *m)
{
  UARTWrite("uIP log message: ");
  UARTWrite(m);
  UARTWrite("\r\n");
}
