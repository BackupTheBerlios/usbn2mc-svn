/*
	atkeyb.h
	
	AT-Keyboard library
	Taken from ATMEL Appication Note AVR313
	
	Modified for use with avrgcc
	
	2/2005 Stefan Seegel dahamm@gmx.net
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/signal.h>
#include "atkeyb.h"

#include "uart.h"

char str[5];

#define PINOFPORT(x) (*(&x - 2))
#define DDROFPORT(x) (*(&x - 1))

// Unshifted characters
unsigned char unshifted[][2] PROGMEM = {
	{0x0d,9}, {0x0e,'|'}, {0x15,'q'}, {0x16,'1'}, {0x1a,'z'}, {0x1b,'s'}, {0x1c,'a'}, {0x1d,'w'},
	{0x1e,'2'}, {0x21,'c'}, {0x22,'x'}, {0x23,'d'}, {0x24,'e'}, {0x25,'4'}, {0x26,'3'}, {0x29,' '},
	{0x2a,'v'}, {0x2b,'f'}, {0x2c,'t'}, {0x2d,'r'}, {0x2e,'5'}, {0x31,'n'}, {0x32,'b'}, {0x33,'h'},
	{0x34,'g'}, {0x35,'y'}, {0x36,'6'}, {0x39,','}, {0x3a,'m'}, {0x3b,'j'}, {0x3c,'u'}, {0x3d,'7'},
	{0x3e,'8'}, {0x41,','}, {0x42,'k'}, {0x43,'i'}, {0x44,'o'}, {0x45,'0'}, {0x46,'9'}, {0x49,'.'},
	{0x4a,'-'}, {0x4b,'l'}, {0x4c,'ø'}, {0x4d,'p'}, {0x4e,'+'}, {0x52,'æ'}, {0x54,'å'}, {0x55,'\\'},
	{0x5a,13}, {0x5b,'¨'}, {0x5d,'\''}, {0x61,'<'}, {0x66,8}, {0x69,'1'}, {0x6b,'4'}, {0x6c,'7'},
	{0x70,'0'}, {0x71,','}, {0x72,'2'}, {0x73,'5'}, {0x74,'6'}, {0x75,'8'}, {0x79,'+'}, {0x7a,'3'},
	{0x7b,'-'}, {0x7c,'*'}, {0x7d,'9'}, {0,0}};

// Shifted characters
unsigned char shifted[][2] PROGMEM = {
	0x0d,9, 0x0e,'§', 0x15,'Q', 0x16,'!', 0x1a,'Z', 0x1b,'S', 0x1c,'A', 0x1d,'W',
	0x1e,'"', 0x21,'C', 0x22,'X', 0x23,'D', 0x24,'E', 0x25,'¤', 0x26,'#', 0x29,' ',
	0x2a,'V', 0x2b,'F', 0x2c,'T', 0x2d,'R', 0x2e,'%', 0x31,'N', 0x32,'B', 0x33,'H',
	0x34,'G', 0x35,'Y', 0x36,'&', 0x39,'L', 0x3a,'M',	0x3b,'J',	0x3c,'U',	0x3d,'/',
	0x3e,'(', 0x41,';',	0x42,'K',	0x43,'I',	0x44,'O',	0x45,'=',	0x46,')',	0x49,':',
	0x4a,'_', 0x4b,'L', 0x4c,'Ø', 0x4d,'P', 0x4e,'?', 0x52,'Æ', 0x54,'Å', 0x55,'`',
	0x5a,13, 0x5b,'^', 0x5d,'*', 0x61,'>', 0x66,8, 0x69,'1', 0x6b,'4', 0x6c,'7',
	0x70,'0', 0x71,',', 0x72,'2', 0x73,'5', 0x74,'6', 0x75,'8', 0x79,'+', 0x7a,'3',
	0x7b,'-', 0x7c,'*', 0x7d,'9', 0,0};


unsigned char edge = 0, bitcount = 11;// 0 = neg. 1 = pos.
unsigned char kb_buffer[BUFF_SIZE];
unsigned char *inpt, *outpt;
volatile unsigned char atkeyb_buffcnt;

void put_kbbuff(unsigned char c);
void decode(unsigned char sc);

void atkeyb_init(void)
{
	inpt = kb_buffer;// Initialize buffer
	outpt = kb_buffer;
	atkeyb_buffcnt = 0;
	DDROFPORT(KEYB_DATA_PORT)&=~(1<<KEYB_DATA_PIN);
	KEYB_DATA_PORT&=~(1<<KEYB_DATA_PIN);
	//MCUCR = 1<<ISC01; // INT0 interrupt on falling edge
	MCUCSR = 0 << ISC2; // falling edge
	GIFR|=1<<INTF2; //Clear interrut 0 flag
	GICR|=1<<INT2;
}

void atkeyb_interrupt()
{
	static unsigned char data;// Holds the received scan code
	if (!edge) // Routine entered at falling edge
	{
		if (bitcount < 11 && bitcount > 2)// Bit 3 to 10 is data. Parity bit,
		{ // start and stop bits are ignored.
			data = (data >> 1);
			if (PINOFPORT(KEYB_DATA_PORT) & (1<<KEYB_DATA_PIN))
				data = data | 0x80;// Store a '1'
		}
		
		//MCUCR = 3;// Set interrupt on rising edge
		MCUCSR = 1 << ISC2; // rising edge 
		edge = 1;
	}
	else
	{ // Routine entered at rising edge
		//MCUCR = 2;// Set interrupt on falling edge
		MCUCSR = 0 << ISC2; // falling edge 
		edge = 0;
		if (--bitcount == 0)// All bits received
		{
			decode(data);
			bitcount = 11;
		}
	}
}

void decode(unsigned char sc)
{
	static unsigned char is_up=0, shift = 0, mode = 0;
	unsigned char i;
	unsigned char a, b;
	if (!is_up)// Last data received was the up-key identifier
	{
		switch (sc)
		{
			case 0xF0 :// The up-key identifier
				is_up = 1;
				break;
			
			case 0x12 :// Left SHIFT
				shift = 1;
				break;

			case 0x59 :// Right SHIFT
				shift = 1;
				break;
				
			case 0x05 :// F1
				if(mode == 0)
					mode = 1;// Enter scan code mode
				if(mode == 2)
					mode = 3;// Leave scan code mode
				break;
				
			default:
				if(mode == 0 || mode == 3)// If ASCII mode
				{
					if(!shift)// If shift not pressed,
					{ // do a table look-up
						i=0;
						a=pgm_read_byte(&unshifted[i][0]);
						while ((a) && (a!=sc))
						{
							i++;
							a=pgm_read_byte(&unshifted[i][0]);
						//SendHex(a);
						}
						
						if (a == sc)
						{
							put_kbbuff(pgm_read_byte(&unshifted[i][1]));
						}
					}
					else
					{// If shift pressed
						i=0;
						a=pgm_read_byte(&shifted[i][0]);
						while ((a) && (a!=sc))
						{
							i++;
							a=pgm_read_byte(&shifted[i][0]);
						}
						
						if (a == sc)
						{
							put_kbbuff(pgm_read_byte(&shifted[i][1]));
						}
					}
				}
				else
				{ // Scan code mode
					//print_hexbyte(sc);// Print scan code
				}
		}
	}
	else
	{
		is_up = 0;// Two 0xF0 in a row not allowed
		switch (sc)
		{
			case 0x12 :// Left SHIFT
				shift = 0;
				break;

			case 0x59 :// Right SHIFT
				shift = 0;
				break;

			case 0x05 :// F1
				if(mode == 1)
					mode = 2;
				if(mode == 3)
					mode = 0;
				break;

			case 0x06 :// F2
				//clr();
				break;
		}
	}
}

void put_kbbuff(unsigned char c)
{
	if (atkeyb_buffcnt<BUFF_SIZE)// If buffer not full
	{
		*inpt = c;// Put character into buffer
		inpt++; // Increment pointer
		atkeyb_buffcnt++;
		if (inpt >= kb_buffer + BUFF_SIZE)// Pointer wrapping
		inpt = kb_buffer;
	}
}

char atkeyb_getchar(void)
{
	unsigned char byte;
	while(atkeyb_buffcnt == 0);// Wait for data
	byte = *outpt;// Get byte
	outpt++; // Increment pointer
	if (outpt >= kb_buffer + BUFF_SIZE)// Pointer wrapping
		outpt = kb_buffer;
	atkeyb_buffcnt--; // Decrement buffer count
	return byte;
}
