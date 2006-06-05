#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "vscope.h"
#include "usbn2mc.h"


#include "fifo.h"

int datatogl=0;
void VScopePingPongTX1()
{
  //UARTWrite("tx1 demo\r\n");
  // zum interrupt zeit messen
  
  int i;
  if(togl==1){
    PORTB = 0xFF;
    togl=0;
  } else {
    PORTB = 0x00;
    togl=1;
  }
  
  // stop if there are no further data
  //if(vscope.fifo.count > 63 )
  if(1)
  { 
    // send next 64 bytes
    USBNWrite(TXC1,FLUSH);
    USBNWrite(TXD1,0x33);
    //USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));
    for(i=1;i<64;i++)
      USBNBurstWrite(0x44);
      //USBNBurstWrite(fifo_get_nowait(&vscope.fifo));

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
  else {
    vscope.update1=1;
    datatogl=0;
    return;
  }
}

void VScopePingPongTX2()
{
  UARTWrite("tx2 demo\r\n");
  USBNWrite(EPC1,EP_EN+3);
  vscope.update2=1;
}


// send data to the application on pc
void VScopeSendScopeData()
{
  int i;
  // interrupts off
  if(vscope.update1==1)
  {
    //if(vscope.fifo.count > 63 )
    //{
      USBNWrite(TXC1,FLUSH);
      USBNWrite(TXD1,0x11);
      //USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));
      for(i=1;i<64;i++)
	USBNBurstWrite(0x22);
	//USBNBurstWrite(fifo_get_nowait(&vscope.fifo));

      USBNWrite(TXC1,TX_LAST+TX_EN);
      vscope.update1=0;
    //}
  }
/*
  if(vscope.update2==1)
  {
    USBNWrite(TXC2,FLUSH);
    USBNWrite(TXD2,fifo_get_nowait(&vscope.fifo));
    for(i=1;i<64;i++)
      USBNBurstWrite(fifo_get_nowait(&vscope.fifo));

    USBNWrite(TXC2,TX_LAST+TX_EN+TX_TOGL);
    vscope.update2=0;
  }
*/
  // global interrupts on
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
      TCCR1B = (1 << WGM12) | (1 << CS12)  | (1 << CS10); //1024tel vom takt 64uS 15K
      // OutputCompare1A Register setzen
      OCR1A = 0x0000;
      OCR1A = 0x0fff;
      // enable interrupt
      TIMSK |= (1 << OCIE1A);
     
    break;
    case CMD_STOPSCOPE:
      UARTWrite("stop scope\r\n");
      TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10); // Stop (Timer/Counter) 
    break;
    case CMD_GETSCOPEMODE:
      UARTWrite("get scope mode\r\n");
      SendHex(vscope.mode);

    break;
    case CMD_GETSCOPESTATE:
      UARTWrite("get scope state\r\n");

    break;
    case CMD_GETFIFOLOAD:
      UARTWrite("get fifo load ");
      SendHex(vscope.fifo.count);
      UARTWrite("\r\n");

    break;
    default:
      UARTWrite("unknown command\r\n");
  }
}


