#include "vprotocol.h"

/* device handle */
struct vport 
{
	struct usb_dev_handle* usb_handle;
};

/**
 * create an device handle for vport
 */
extern struct vport* vport_open();


/**
 * Sends a message to vport
 * param vport = device handle
 * param msg = pointer to a message
 * param msglen = length of message
 * param buf = pointer for message result
 * param buflen = max length for result message
 */
extern int vport_message(struct vport *vport, char *msg, int msglen);



/**
 * close connection to usb device
 */
extern int vport_close(struct vport *vport);


 /**
 * Setupfunction for serial 2 usb / usb 2 serial converter 
 * param vport = device handle
 * param port = portnumber of uart in vport device
 * param flags = configuration for uart
 */
extern int vport_serial_configuration(struct vport *vport, int port, char flags);
extern int vport_serial_read(struct vport *vport, int port, char* buf, int size);
extern int vport_serial_write(struct vport *vport, int port, char* buf, int size);









extern int vport_port_direction(struct vport *vport, int port, int value);
extern int vport_port_set(struct vport *vport, int port, int value);
extern int vport_port_get(struct vport *vport, int port);




