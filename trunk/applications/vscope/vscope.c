#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "vscope.h"
#include "usbn2mc.h"

void VScopePingPongTX1()
{
	
}

void VScopePingPongTX2()
{
	
}


// send data to the application on pc
void VScopeSendScopeData()
{
  UARTWrite("data can be send\n");
  /*
  int i;
  USBNWrite(TXC1,FLUSH);     
  //USBNWrite(TXD1,buf[0]);
  //for(i=1;i<64;i++)
  //  USBNBurstWrite(buf[i]);

  if(togl==0)
  {
    USBNWrite(TXC1,TX_LAST+TX_EN);
    togl=1;
  } else 
  {
    USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
    togl=0;
  }
  */
}


// get and extract commands from the application on the pc
void VScopeCommand(char *buf)
{

  switch(buf[0])
  { 
    case CMD_SETMODE:
      UARTWrite("set mode ");
      SendHex(buf[2]);
      UARTWrite("\r\n");
    break;
    case CMD_STARTSCOPE:
      UARTWrite("start scope\r\n");
      
      TCCR1A = 0;
      // 16 MHz / 64 = 250K = 4us
      TCCR1B = (1 << WGM12) | (1 << CS12)  | (1 << CS10); //1024tel vom takt 64uS
      // OutputCompare1A Register setzen
      OCR1A = 0x0000;
      OCR1A = 0x1fff;
      // enable interrupt
      TIMSK |= (1 << OCIE1A);
     
    break;
    case CMD_STOPSCOPE:
      UARTWrite("stop scope\r\n");
      TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10); // Stop (Timer/Counter) 
    break;
    case CMD_GETSCOPEMODE:
      UARTWrite("get scope mode\r\n");

    break;
    case CMD_GETSCOPESTATE:
      UARTWrite("get scope state\r\n");

    break;
    case CMD_GETFIFOLOAD:
      UARTWrite("get fifo load\r\n");

    break;
    default:
      UARTWrite("unknown command\r\n");
  }
}


