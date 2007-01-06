#include "vport.h"
#include "vprotocol.h"
#include <usb.h>

struct vport* vport_open()
{
	struct usb_bus *busses;
	struct usb_dev_handle* usb_handle;
	struct usb_bus *bus;
	struct usb_device *dev;

	struct vport * tmp;

	tmp = (struct vport*)malloc(sizeof(struct vport));
	
	
	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();		

	/* find vport device in usb bus */

	for (bus = busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			/* condition for sucessfully hit (too bad, I only check the vendor id)*/
			if (dev->descriptor.idVendor == 0x0400)
			{
				tmp->usb_handle = usb_open(dev);
				
				usb_set_configuration (tmp->usb_handle,dev->config[0].bConfigurationValue);
				usb_claim_interface(tmp->usb_handle, 0);
				usb_set_altinterface(tmp->usb_handle,0);

				return tmp;
			}
				
		}
	}
	return 0;	
}


int vport_close(struct vport *vport)
{
	usb_close(vport->usb_handle);
	free(vport);
}



int vport_message(struct vport *vport, char *msg, int msglen)
{
	return usb_bulk_write(vport->usb_handle,2,msg,msglen,1);
}



int vport_serial_configuration(struct vport *vport, int port, char flags)
{
	// build and send an vport message
	char buf[4];
	buf[0]=PART_SERIAL;		// interface
	buf[1]=4;				// length of msg
	buf[2]=port;			// port number
	buf[3]=flags;			// configuration
	
	vport_message(vport, buf, 4);

}

int vport_serial_read(struct vport *vport, int port, char* buf, int size)
{

}

int vport_serial_write(struct vport *vport, int port, char* buf, int size)
{
	char usbbuf[64];

	usbbuf[0]=PART_SERIAL;			// cmd uart/serial::
	usbbuf[1]=size+4;				// length of message
	usbbuf[2]=1;					// port number 
	usbbuf[3]=CMD_SERIAL_CONFIG;	// sending command

	int i;
	for(i=0;i<size;i++)
		usbbuf[i+4] = buf[i];
	
	vport_message(vport, usbbuf, usbbuf[1]);
}





int vport_port_direction(struct vport *vport, int port, int value)
{

}

int vport_port_set(struct vport *vport, int port, int value)
{

}

int vport_port_get(struct vport *vport, int port)
{

}

