#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "vscope.h"
#include "usbn2mc.h"



// send data to the application on pc
void VscopeSendData(char *buf)
{
  int i;
  USBNWrite(TXC1,FLUSH);     
  USBNWrite(TXD1,buf[0]);
  for(i=1;i<64;i++)
    USBNBurstWrite(buf[i]);

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


// get and extract commands from the application on the pc
void VScopeCommand(char *buf)
{
  /*
   * start/stop logik analyser
   *
   * start/stop 1 channel scope
   * start/stop 2 channel scope
   * start/stop 4 channel scope
   * start/stop 8 channel scope
   * 
   * set/get ioport
   */
   UARTWrite("command\r\n");
   //bufptr=buffer;
   //bufindex=0;
   TCCR1A = 0;
   // 16 MHz / 64 = 250K = 4us
   TCCR1B = (1 << WGM12) | (1 << CS12)  | (0 << CS10);

   // OutputCompare1A Register setzen
   OCR1A = 0x0000;

   // enable interrupt
   TIMSK |= (1 << OCIE1A);
}


