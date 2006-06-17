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
  if(vscope.state!=STATE_RUNNING)
    return;
  // stop if there are no further data
  //if(vscope.fifo.count > 63 )
  int i;
  if(1)
  { 
    // send next 64 bytes
    USBNWrite(TXC1,FLUSH);
    // ******** wait on timer condition and clear condition
    //_wait_spinlock();
    USBNWrite(TXD1,PINB);
    //USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));
    for(i=1;i<64;i++)
    {
      // ******** wait on timer condition and clear condition
      //_wait_spinlock();
      USBNBurstWrite(PINB);
    }
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
      // ******** wait on timer condition and clear condition
      //_wait_spinlock();
      USBNWrite(TXD1,PINB);
      //_wait_spinlock();
      //USBNWrite(TXD2,PINB);
      //USBNBurstWrite(PINB);
      //_wait_spinlock();
      //USBNBurstWrite(PINB);
      //_wait_spinlock();
      //USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));
      for(i=1;i<64;i++)
      {
	// ******** wait on timer condition and clear condition
	//_wait_spinlock();
	USBNBurstWrite(PINB);
	//USBNBurstWrite(fifo_get_nowait(&vscope.fifo));
      }

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
      vscope.spinlock=0xff;	    
      UARTWrite("set mode ");
      SendHex(buf[2]);
      UARTWrite("\r\n");
    break;

    case CMD_SETSAMPLERATE:
      //UARTWrite("set mode ");
      //SendHex(buf[2]);
      //UARTWrite("\r\n");
      vscope.samplerate=buf[2];
    break;

    case CMD_STARTSCOPE:
      UARTWrite("start scope\r\n");
      datatogl=0;
      vscope.update1=1;
      //vscope.update2=1;

      TCCR1A = 0;
      // 16 MHz / 64 = 250K = 4us
      //TCCR1B = (1 << WGM12) | (1 << CS12)  | (1 << CS10); //1024tel vom takt 64uS 15K
      switch(vscope.samplerate)
      {
	case SAMPLERATE_5US:
	  UARTWrite("5us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 10; //10 * 500ns = 5us
	break;
	case SAMPLERATE_100US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 200; //200 * 500ns = 100us
	break;
	case SAMPLERATE_1MS:
	  UARTWrite("1ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 2000; //200 * 500ns = 1ms
	break;
	case 0x09:
	  UARTWrite("1ms\n\r");
	  //TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  TCCR1B = (1 << 3) | (1 << CS12)  | (1 << CS10); //8tel vom takt = 500ns 
	  OCR1A = 15000; //200 * 500ns = 1ms
	break;
	default:
	  UARTWrite("default\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns
	  OCR1A = 2000; //200 * 500ns = 1ms
      }
      // enable interrupt
      TIMSK |= (1 << OCIE1A);
      vscope.state=STATE_RUNNING;
      VScopeSendScopeData();
    break;

    case CMD_STOPSCOPE:
      vscope.state=STATE_DONOTHING;
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


void VScopeNothing(char *buf)
{
  if(buf[2]==1)
  {
    __asm__ __volatile ("; nur ein asm-Kommentar");
  }
}
void _wait_spinlock()
{
  while(1)
  {
    if(vscope.spinlock)
    {
      VScopeNothing(NULL);
      //UARTWrite("");
      vscope.spinlock=0;
      //return;
      break;
    }
  }
}
					      //
