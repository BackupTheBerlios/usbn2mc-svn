/*
	atkeyb.h
	
	AT-Keyboard library
	Taken from ATMEL Appication Note AVR313
	
	Modified for use with avrgcc
	
	2/2005 Stefan Seegel dahamm@gmx.net
	
	Connection to at-keyboard:
		Clock-Line -> interrupt 0
		Data-Line -> any IO pin (set below)
		
	Preparation in your application:
		-set interrupts active: sei();
		-initialize at-keyboard lib: atkeyb_init();
*/

#define BUFF_SIZE 64					//max. size of keyboard buffer
#define KEYB_DATA_PORT PORTA			//PORT which data-pin is connected to
#define KEYB_DATA_PIN PA0		 		//PIN of data-pin

extern volatile unsigned char atkeyb_buffcnt;	//amount of bytes in buffer
extern void atkeyb_interrupt();
extern void atkeyb_init(void);
/*
	call init_kb once at startup
*/

extern char atkeyb_getchar(void);						 	//
/*
	returns char from keyboard buffer
	this function doesn't return until at least 1 char is in buffer
	Maybe you might check atkeyb_buffcnt to check if data is in buffer!
*/
