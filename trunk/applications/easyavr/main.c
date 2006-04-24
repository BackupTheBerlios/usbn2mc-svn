#include <stdlib.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
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


uint8_t state;
uint8_t page_addr;


#define NONE	    0x00
#define STARTAPP    0x01
#define WRITEPAGE   0x02
#define READCHKSUM  0x03


// pointer to the beginning of application code
void (*jump_to_app)( void ) = 0x0000;


void BootProgramPage (uint32_t page, uint8_t *buf)
{
  uint16_t i;
  uint8_t sreg;

  // Disable interrupts.

  sreg = SREG;
  cli();
    
  eeprom_busy_wait ();

  boot_page_erase (page);
  boot_spm_busy_wait ();      // Wait until the memory is erased.

  for (i=0; i<SPM_PAGESIZE; i+=2)
  {
    // Set up little-endian word.

    uint16_t w = *buf++;
    w += (*buf++) << 8;
        
    boot_page_fill (page + i, w);
  }

  boot_page_write (page);     // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.

  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).

  SREG = sreg;
}



// start programm from application sector
void BootLoaderRunApplication()
{
  USBNWrite(RXC1,FLUSH);
  USBNWrite(RXC1,RX_EN);

  USBNWrite(MCNTRL,SRST);  // clear all usb registers

  MCUCR = (1<<IVCE);       // move interruptvectors to the Application sector
  MCUCR = (0<<IVSEL);      // device specific !
  jump_to_app();	  // Jump to application sector
}


void BootLoader(char *buf)
{
  // check state first ist 
  switch(state)
  {
    case WRITEPAGE:
      //UARTWrite("write\r\n");
      //SendHex(page_addr);
      BootProgramPage (page_addr,buf);
      state = NONE;
    break;
    default:
      //UARTWrite("default\r\n");
      state = buf[0];
      page_addr = buf[1];

      if(state==STARTAPP)
      {
	cli();
	UARTWrite("start\r\n");
	BootLoaderRunApplication();
	state = NONE;
      }

  }
	
}


int main(void)
{

  // spm (bootloader mode from avr needs this, to use an own isr table)	
  cli();
  GICR = _BV(IVCE);  // enable wechsel der Interrupt Vectoren
  GICR = _BV(IVSEL); // Interrupts auf Boot Section umschalten
  sei();


  // bootloader application starts here

  const unsigned char easyavrDevice[] =
  { 0x12,	      // 18 length of device descriptor
    0x01,       // descriptor type = device descriptor 
    0x10,0x01,  // version of usb spec. ( e.g. 1.1) 
    0x00,	      // device class
    0x00,	      // device subclass
    0x00,       // protocol code
    0x08,       // deep of ep0 fifo in byte (e.g. 8)
    0x00,0x04,  // vendor id
    0x12,0x34,  // product id
    0x03,0x01,  // revision id (e.g 1.02)
    0x00,       // index of manuf. string
    0x00,	      // index of product string
    0x00,	      // index of ser. number
    0x01        // number of configs
  };

  // ********************************************************************
  // configuration descriptor          
  // ********************************************************************

  const unsigned char easyavrConf[] =
  { 0x09,	      // 9 length of this descriptor
    0x02,       // descriptor type = configuration descriptor 
    0x20,0x00,  // total length with first interface ... 
    0x01,	      // number of interfaces
    0x01,	      // number if this config. ( arg for setconfig)
    0x00,       // string index for config
    0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    0x1A,        // power for this configuration in mA (e.g. 50mA)
    //InterfaceDescriptor
    0x09,	      // 9 length of this descriptor
    0x04,       // descriptor type = interface descriptor 
    0x00,	      // interface number 
    0x00,	      // alternate setting for this interface 
    0x02,	      // number endpoints without 0
    0x00,       // class code 
    0x00,       // sub-class code 
    0x00,       // protocoll code
    0x00,       // string index for interface
    //EP1 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x81,	      // endpoint address (e.g. in ep1)
    0x02,	      // transfer art ( bulk )
    0x80,0x00,  // fifo size
    0x00,	      // polling intervall in ms
    //EP2 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x02,	      // endpoint address (e.g. out ep2)
    0x02,	      // transfer art ( bulk )
    0x00,0x08,  // fifo size
    0x00	      // polling intervall in ms
  };

  UARTInit();

  USBNInit(easyavrDevice,easyavrConf);   

  USBNCallbackFIFORX1(&BootLoader);

  //MCUCR |=  (1 << ISC01); // fallende flanke
  //GICR |= (1 << INT0);

  USBNInitMC();

  // start usb chip
  USBNStart();
  
  UARTWrite("\r\nbootloader is now active\r\n");

  while(1);
}

