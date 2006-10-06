#include <usb.h>


struct backpanel
{
	struct usb_dev_handle* usb_handle;
};

	
int bp_open(struct backpanel *bp);

int bp_serial_configuration(int port, long baud, int parity, int bits, int stopbits);

int bp_serial_read(int port, char* buf, int size);

int bp_serial_write(int port, char* buf, int size);
