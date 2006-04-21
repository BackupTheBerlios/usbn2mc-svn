#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <avr/delay.h>

#include "uart.h"
#include "usbn2mc.h"

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



void At89SPIOut(char data)
{
  SPDR = data;
  while ( !(SPSR & (1<<SPIF)) ) ;
}


void At89WriteCode(int addr, char data)
{
  At89SPIOut(0x02 | ((addr >> 5) & 0xF8) | ((addr >> 11) & 0x04)); /* hhhh h010 */
  At89SPIOut(addr & 0xFF); /* llll llll */
  At89SPIOut(data);
  wait_ms(3);
}


void At89FlashWrite(char *buf)
{
  int startaddr,len;
  //UARTWrite("start upload\r\n");
  // hex file to spi interface
  DDRB=0xff;
  
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

  PORTB=0x10;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x00;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x10;  // reset on led on and sck = low 


  //before the at89 accepts any commands, it needs to be put into command mode
  At89SPIOut(0xAC);
  At89SPIOut(0x53);
  At89SPIOut(0x00);
  wait_ms(9);

  //74 00 f5 90  7a ff 7b 14  db fe da fa  04 80 f3
  //load programm into flash
  int i; 
  startaddr = (int)buf[1]; //high 
  startaddr = startaddr << 8;//high 
  startaddr = startaddr + (int)buf[2]; // low

  for(i=0;i<(int)buf[3];i++)
  {
    At89WriteCode(startaddr+i,buf[i+4]); 
    //SendHex(startaddr+i);
    SendHex(buf[i+4]);
    //USBNDebug("\r\n");
  }
    USBNDebug("\r\n");


  wait_ms(9);
  PORTB=0x02;
  //UARTWrite("ready...\r\n");

}

void At89FlashErase()
{
  int i,j;
  // hex file to spi interface
  DDRB=0xff;
  
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

  PORTB=0x10;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x00;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x10;  // reset on led on and sck = low 


  //before the at89 accepts any commands, it needs to be put into command mode
  At89SPIOut(0xAC);
  At89SPIOut(0x53);
  At89SPIOut(0x00);
  wait_ms(9);

  SPDR = 0xAC;
  At89SPIOut(0xAC);
  At89SPIOut(0x04);
  At89SPIOut(0x00);
  wait_ms(16);
  
  PORTB=0x02;
  UARTWrite("ready...\r\n");

}


void At89Flash(char *buf)
{

  if(buf[0]==0x01)
    At89FlashErase();
  else if(buf[0]==0x02)
    At89FlashWrite(buf);
  else {}
}

int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x9876);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("usbprogrammer");
  USBNDeviceSerialNumber("2006-04-12");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&At89Flash);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0);

  
  MCUCR |=  (1 << ISC01); // fallende flanke

  GICR |= (1 << INT0);
  sei();
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
}

