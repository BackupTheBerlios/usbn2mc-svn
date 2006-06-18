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
  vscope.tx=1;
}

void VScopePingPongTX2()
{
  UARTWrite("tx2 demo\r\n");
  USBNWrite(EPC1,EP_EN+3);
  //vscope.update2=1;
}

// send data to the application on pc
void VScopeSendScopeData()
{
  //UARTWrite("read data\r\n");
  int i;
  USBNWrite(TXC1,FLUSH);
  
  USBNWrite(TXD1,fifo_get_nowait(&vscope.fifo));
  for(i=1;i<64;i++)
      USBNBurstWrite(fifo_get_nowait(&vscope.fifo));
  USBNWrite(TXC1,TX_LAST+TX_EN);
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

    case CMD_SETSAMPLERATE:
      //UARTWrite("set mode ");
      //SendHex(buf[2]);
      //UARTWrite("\r\n");
      vscope.samplerate=buf[2];
    break;

    case CMD_STARTSCOPE:
      UARTWrite("start scope\r\n");
      datatogl=0;
      //vscope.update2=1;
      fifo_init(&vscope.fifo, fifobuffer, 1000);


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
	case SAMPLERATE_10US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 20; //200 * 500ns = 100us
	break;
	case SAMPLERATE_50US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 100; //200 * 500ns = 100us
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
	case SAMPLERATE_10MS:
	  UARTWrite("10ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS12) ; //6250 *16M/256 = 100ms 
	  OCR1A = 625;
	break;
	case SAMPLERATE_100MS:
	  UARTWrite("100ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS12) ; //6250 *16M/256 = 100ms 
	  OCR1A = 6250;
	break;
	default:
	  UARTWrite("default\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns
	  OCR1A = 2000; //200 * 500ns = 1ms
      }
      // enable interrupt
      TIMSK |= (1 << OCIE1A);
      vscope.state=STATE_RUNNING;
    break;

    case CMD_STOPSCOPE:
      vscope.fifo.count=0;
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

    case CMD_GETDATA:
      //UARTWrite("get scope state\r\n");
      //VScopeSendScopeData();
      VScopePingPongTX1();
    break;


    default:
      UARTWrite("unknown command\r\n");
  }
}

