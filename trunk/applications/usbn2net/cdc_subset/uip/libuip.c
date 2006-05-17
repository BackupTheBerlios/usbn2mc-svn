/*
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: main.c,v 1.10.2.1 2003/10/04 22:54:17 adam Exp $
 *
 */


#include "uip/uip.h"
#include "uip/uip_arp.h"

#include <avr/signal.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

/*-----------------------------------------------------------------------------------*/
void libuip_init(void)
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



void libuip_reveicepacketdata(char *data,int length)
{
  int i;
  //uip_log("get data");
  // add data to tcp stack
  for(i=0;i<length;i++){
    uip_buf[i]=data[i];
    //SendHex(uip_buf[i]);
  }
  uip_len = uip_len + length;
}


void libuip_send()
{
 TX(uip_buf,uip_len); 
 //TX(uip_buf,42); 
    
 uip_len=0;


}

void libuip_loop(void)
{
  u8_t i, arptimer;
  arptimer=0;

  while(1) {
    cli();
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
	  libuip_send();
	}
      }
      /* Call the ARP timer function every 10 seconds. */
      
      if(++arptimer == 20) {	
	uip_arp_timer();
	arptimer = 0;
      }
      
    } else {

      if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	uip_log("eth found");
	uip_arp_ipin();
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  //tapdev_send();
	  libuip_send();
	}
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_log("arp found");
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */	
	if(uip_len > 0) {	
	  //tapdev_send();
	  //SendHex(uip_len+30);
	  libuip_send();
	}
      }
    }
     sei();
  }
}


void uip_log(char *m)
{
  UARTWrite("uIP log message: ", m);
  UARTWrite(m);
  UARTWrite("\r\n");
}
