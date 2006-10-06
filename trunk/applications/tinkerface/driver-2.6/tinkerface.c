/*
|-----------------------------------------------------------------------------------------
| License:
| This program is free software; you can redistribute it and/or modify it under
| the terms of the GNU General Public License as published by the Free Software
| Foundation; either version 2 of the License, or (at your option) any later
| version.
| This program is distributed in the hope that it will be useful, but
|
| WITHOUT ANY WARRANTY;
|
| without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
| PURPOSE. See the GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License along with
| this program; if not, write to the Free Software Foundation, Inc., 51
| Franklin St, Fifth Floor, Boston, MA 02110, USA
|
| http://www.gnu.de/gpl-ger.html
`-----------------------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <asm/uaccess.h>
#include "usb-serial.h"

static int debug;

static int tinkerface_attach(struct usb_serial *serial);
static void tinkerface_set_termios(struct usb_serial_port *port, struct termios * old);
static int tinkerface_ioctl (struct usb_serial_port *port, struct file *file, unsigned int cmd, unsigned long arg);
static int tinkerface_tiocmget (struct usb_serial_port *port, struct file *file);
static int tinkerface_open (struct usb_serial_port *port, struct file *filp);
static void tinkerface_read_bulk_callback (struct urb *urb, struct pt_regs *regs);

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(0x6547, 0x0232) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver tinkerface_driver = {
	.name = "tinkerface",
	.probe = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table = id_table,
};

static struct usb_serial_driver tinkerface_device = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "tinkerface",
	},
	.id_table = id_table,
	.num_interrupt_in = 1,
	.num_bulk_in = 1,
	.num_bulk_out = 1,
	.num_ports = 1,
	.attach = tinkerface_attach,
	.set_termios = tinkerface_set_termios,
	.ioctl = tinkerface_ioctl,
	.tiocmget = tinkerface_tiocmget,
	.open = tinkerface_open,
};

struct tinkerface_private {
	spinlock_t lock;
	u8 termios_initialized;
};


#define ARK3116_SND(seq, a,b,c,d) { rst = usb_control_msg(serial->dev,usb_sndctrlpipe(serial->dev,0), a,b,c,d,NULL,0x00, 1000); dbg("%03d > ok",seq); }
#define ARK3116_RCV(seq, a,b,c,d, expected) {rst = usb_control_msg(serial->dev,usb_rcvctrlpipe(serial->dev,0),a,b,c,d,buf,0x0000001, 1000);  if (rst) dbg("%03d < %d bytes [0x%02X] expected [0x%02X]",seq,rst,buf[0],expected); else dbg("%03d < 0 bytes", seq);}
#define ARK3116_RCV_QUIET(a,b,c,d){rst = usb_control_msg(serial->dev,usb_rcvctrlpipe(serial->dev,0),a,b,c,d,buf,0x0000001, 1000);}

static int tinkerface_attach(struct usb_serial *serial) {
	char *buf;
	struct tinkerface_private *priv;
	int rst;
	int i;
	rst=0;
	
	for (i = 0; i < serial->num_ports; ++i) {
		priv = kmalloc (sizeof (struct tinkerface_private), GFP_KERNEL);
		if (!priv)
			goto cleanup;
		memset (priv, 0x00, sizeof (struct tinkerface_private));
		spin_lock_init(&priv->lock);
		
		usb_set_serial_port_data(serial->port[i], priv);
	}
	
	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc -> out of mem ?");
		goto cleanup;
	}

	//3
	ARK3116_SND( 3,0xFE,0x40,0x0008,0x0002);
	ARK3116_SND( 4,0xFE,0x40,0x0008,0x0001);
	ARK3116_SND( 5,0xFE,0x40,0x0000,0x0008);
	ARK3116_SND( 6,0xFE,0x40,0x0000,0x000B);
	
	//<-- seq7
	ARK3116_RCV( 7,0xFE,0xC0,0x0000,0x0003, 0x00);
	ARK3116_SND( 8,0xFE,0x40,0x0080,0x0003);
	ARK3116_SND( 9,0xFE,0x40,0x001A,0x0000);
	ARK3116_SND(10,0xFE,0x40,0x0000,0x0001);
	ARK3116_SND(11,0xFE,0x40,0x0000,0x0003);
	
	//<-- seq12
	ARK3116_RCV(12,0xFE,0xC0,0x0000,0x0004, 0x00);
	ARK3116_SND(13,0xFE,0x40,0x0000,0x0004);

	//14
	ARK3116_RCV(14,0xFE,0xC0,0x0000,0x0004, 0x00);
	ARK3116_SND(15,0xFE,0x40,0x0000,0x0004);

	//16
	ARK3116_RCV(16,0xFE,0xC0,0x0000,0x0004, 0x00);
	//--> seq17
	ARK3116_SND(17,0xFE,0x40,0x0001,0x0004);

	//<-- seq18
	ARK3116_RCV(18,0xFE,0xC0,0x0000,0x0004, 0x01);

	//--> seq19
	ARK3116_SND(19,0xFE,0x40,0x0003,0x0004);


	//<-- seq20
	//seems like serial port status info (RTS, CTS,...)
	ARK3116_RCV(20,0xFE,0xC0,0x0000,0x0006, 0xFF); //returns modem control line status ?!

	//set 9600 baud & do some init ?!
	ARK3116_SND(147,0xFE,0x40,0x0083,0x0003);
	ARK3116_SND(148,0xFE,0x40,0x0038,0x0000);
	ARK3116_SND(149,0xFE,0x40,0x0001,0x0001);
	ARK3116_SND(150,0xFE,0x40,0x0003,0x0003);
	ARK3116_RCV(151,0xFE,0xC0,0x0000,0x0004,0x03);
	ARK3116_SND(152,0xFE,0x40,0x0000,0x0003);
	ARK3116_RCV(153,0xFE,0xC0,0x0000,0x0003,0x00);
	ARK3116_SND(154,0xFE,0x40,0x0003,0x0003);
	
	kfree(buf);
	return(0);
	
cleanup:
	for (--i; i>=0; --i) {
		usb_set_serial_port_data(serial->port[i], NULL);
	}
	return -ENOMEM;	
}


//strange, if this is used something goes wrong and the port is unusable 
//maybe timing problem ?!
//
static int tinkerface_open (struct usb_serial_port *port, struct file *filp){
	struct termios tmp_termios;
	struct usb_serial *serial = port->serial;
	int rst;
	char *buf;
	int result = 0;

	dbg("%s -  port %d", __FUNCTION__, port->number);

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc -> out of mem ?");
	        return -ENOMEM;
	}


	//init stolen from generic.c:
	
	/* force low_latency on so that our tty_push actually forces the data through,
	 * otherwise it is scheduled, and with high data rates (like with OHCI) data
	 * can get lost. */
	if (port->tty)
		port->tty->low_latency = 1;

	/* if we have a bulk interrupt, start reading from it */
	if (serial->num_bulk_in) {
		/* Start reading from the device */
		usb_fill_bulk_urb (port->read_urb, serial->dev,
				usb_rcvbulkpipe(serial->dev, port->bulk_in_endpointAddress),
				port->read_urb->transfer_buffer,
				port->read_urb->transfer_buffer_length,
				((serial->type->read_bulk_callback) ?
				 serial->type->read_bulk_callback :
				 tinkerface_read_bulk_callback),
				port);
		result = usb_submit_urb(port->read_urb, GFP_KERNEL);
		if (result)
			dev_err(&port->dev, "%s - failed resubmitting read urb, error %d\n", __FUNCTION__, result);
	}

	//open
	ARK3116_RCV( 111,0xFE,0xC0,0x0000,0x0003, 0x02); //returns 2
	
	ARK3116_SND( 112,0xFE,0x40,0x0082,0x0003);	
	ARK3116_SND( 113,0xFE,0x40,0x001A,0x0000);
	ARK3116_SND( 114,0xFE,0x40,0x0000,0x0001);
	ARK3116_SND( 115,0xFE,0x40,0x0002,0x0003);
	
	ARK3116_RCV( 116,0xFE,0xC0,0x0000,0x0004, 0x03); //returns 3
	ARK3116_SND( 117,0xFE,0x40,0x0002,0x0004);
	
	ARK3116_RCV( 118,0xFE,0xC0,0x0000,0x0004, 0x02); //returns 2
	ARK3116_SND( 119,0xFE,0x40,0x0000,0x0004);

	ARK3116_RCV( 120,0xFE,0xC0,0x0000,0x0004, 0x00); //returns 0

	ARK3116_SND( 121,0xFE,0x40,0x0001,0x0004);

	ARK3116_RCV( 122,0xFE,0xC0,0x0000,0x0004, 0x01); //returns 1
	
	ARK3116_SND( 123,0xFE,0x40,0x0003,0x0004);
	
	ARK3116_RCV( 124,0xFE,0xC0,0x0000,0x0006, 0xFF); //returns different values (control lines ?!)
	
	//initialise termios:
	if (port->tty) {
		tinkerface_set_termios (port, &tmp_termios);
	}

	kfree(buf);

	//ok
	return(result);
}
	

static void tinkerface_set_termios (struct usb_serial_port *port, struct termios *old_termios){
	/* tinkerface_set_termios */
	struct usb_serial *serial = port->serial;
        struct tinkerface_private *priv = usb_get_serial_port_data(port);
	unsigned int cflag = port->tty->termios->c_cflag;
	unsigned long flags;
	int baud;
	int tinkerface_baud;
	char *buf;
	char config;
	int rst;
	
	rst=0;
	config=0;	

	//dbg ("tinkerface_set_termios port %d", port->number);
	dbg("%s - port %d", __FUNCTION__, port->number);

	if ((!port->tty) || (!port->tty->termios)) {
		dbg("%s - no tty structures", __FUNCTION__);
		return;
	}

	spin_lock_irqsave(&priv->lock, flags);
	if (!priv->termios_initialized) {
		*(port->tty->termios) = tty_std_termios;
		port->tty->termios->c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
		priv->termios_initialized = 1;
	}
	spin_unlock_irqrestore(&priv->lock, flags);
	
	cflag = port->tty->termios->c_cflag;
	
	//check that they really want us to change something:
	if (old_termios) {
		if ((cflag == old_termios->c_cflag)){
			//&& (RELEVANT_IFLAG(port->tty->termios->c_iflag) == RELEVANT_IFLAG(old_termios->c_iflag))) {
			dbg("%s - nothing to change...", __FUNCTION__);
			return;
		}
	}

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc");
		return;
	}


	//set data bit count (8/7/6/5)
	if (cflag & CSIZE){
		switch (cflag & CSIZE){
			case CS5: config |= 0x00; dbg ("setting CS5"); break;
			case CS6: config |= 0x01; dbg ("setting CS6"); break;
			case CS7: config |= 0x02; dbg ("setting CS7"); break;
			default:  err ("CSIZE was set but not CS5-CS8, using CS8!");
			case CS8: config |= 0x03; dbg ("setting CS8"); break;
		}
	}

	//set parity (NONE,EVEN,ODD)
	if (cflag & PARENB){
		if (cflag & PARODD){
			config |= 0x08;
			dbg ("setting parity to ODD");
		}else{
			config |= 0x18;
			dbg ("setting parity to EVEN");
		}
	}else{
		dbg ("setting parity to NONE");
	}

	//SET STOPBIT (1/2)
	if (cflag & CSTOPB){
		config |= 0x04;
		dbg ("setting 2 stop bits");
	}else{
		dbg ("setting 1 stop bit");
	}

	
	//set baudrate:
	baud = 0;
	switch (cflag & CBAUD){
		case B0:        err ("can't set 0baud, using 9600 instead");  break;
		case B75:       baud = 75;      break;
		case B150:      baud = 150;     break;
		case B300:      baud = 300;     break;
		case B600:      baud = 600;     break;
		case B1200:     baud = 1200;    break;
		case B1800:     baud = 1800;    break;
		case B2400:     baud = 2400;    break;
		case B4800:     baud = 4800;    break;
		case B9600:     baud = 9600;    break;
		case B19200:    baud = 19200;   break;
		case B38400:    baud = 38400;   break;
		case B57600:    baud = 57600;   break;
		case B115200:   baud = 115200;  break;
		case B230400:   baud = 230400;  break;
		case B460800:   baud = 460800;  break;
		default:	dbg ("tinkerface driver does not support the baudrate requested (fix it)"); break;
	}
	
	//set 9600 as default (if given baudrate is invalid for example)
	if (baud==0)
		baud = 9600;

	//found by try'n'error, be careful, maybe there are other options for multiplicator etc!
	if (baud == 460800)
		tinkerface_baud = 7; //strange, for 460800 the formula is wrong (dont use round(), then 9600baud is wrong)
	else
		tinkerface_baud = 3000000 / baud;

	//?
	ARK3116_RCV(000,0xFE,0xC0,0x0000,0x0003, 0x03);
	//offset = buf[0];
	//offset = 0x03;
	//dbg("using 0x%04X as target for 0x0003:",0x0080+offset);
	
	
	//set baudrate
	dbg ("setting baudrate to %d (->reg=%d)",baud,tinkerface_baud);
	ARK3116_SND(147,0xFE,0x40,0x0083,0x0003);
	ARK3116_SND(148,0xFE,0x40,(tinkerface_baud & 0x00FF)   ,0x0000);
	ARK3116_SND(149,0xFE,0x40,(tinkerface_baud & 0xFF00)>>8,0x0001);
	ARK3116_SND(150,0xFE,0x40,0x0003,0x0003);

	//?
	ARK3116_RCV(151,0xFE,0xC0,0x0000,0x0004,0x03);
	ARK3116_SND(152,0xFE,0x40,0x0000,0x0003);

	//set data bit count, stop bit count & parity:
	dbg ("updating bit count, stop bit or parity (cfg=0x%02X)", config);
	ARK3116_RCV(153,0xFE,0xC0,0x0000,0x0003,0x00);
	ARK3116_SND(154,0xFE,0x40,config,0x0003);
	
	if (cflag & CRTSCTS){
		dbg ("CRTSCTS not supported by chipset ?!");
	}

	//TEST//ARK3116_SND(154,0xFE,0x40,0xFFFF, 0x0006);
	
	kfree(buf);
	return;
}

//stolen from generic.c
static void tinkerface_read_bulk_callback (struct urb *urb, struct pt_regs *regs){
        struct usb_serial_port *port = (struct usb_serial_port *)urb->context;
        struct usb_serial *serial = port->serial;
        struct tty_struct *tty;
        unsigned char *data = urb->transfer_buffer;
        int result;

        dbg("%s - port %d", __FUNCTION__, port->number);

        if (urb->status) {
                dbg("%s - nonzero read bulk status received: %d", __FUNCTION__, urb->status);
                return;
        }

        usb_serial_debug_data(debug, &port->dev, __FUNCTION__, urb->actual_length, data);

        tty = port->tty;
        if (tty && urb->actual_length) {
                tty_buffer_request_room(tty, urb->actual_length);
		tty_insert_flip_string(tty, data, urb->actual_length);
		tty_flip_buffer_push(tty);
        }

        /* Continue trying to always read  */
        usb_fill_bulk_urb (port->read_urb, serial->dev,
                           usb_rcvbulkpipe (serial->dev,
                                            port->bulk_in_endpointAddress),
                           port->read_urb->transfer_buffer,
                           port->read_urb->transfer_buffer_length,
                           ((serial->type->read_bulk_callback) ?
                             serial->type->read_bulk_callback :
                             tinkerface_read_bulk_callback), port);
        result = usb_submit_urb(port->read_urb, GFP_ATOMIC);
        if (result)
                dev_err(&port->dev, "%s - failed resubmitting read urb, error %d\n", __FUNCTION__, result);
}

static int tinkerface_ioctl (struct usb_serial_port *port, struct file *file, unsigned int cmd, unsigned long arg){
	dbg ("ioctl not supported yet...");
	return -ENOIOCTLCMD;
}

static int tinkerface_tiocmget (struct usb_serial_port *port, struct file *file){
	struct usb_serial *serial = port->serial;
	char buf[1];
	int rst;
	
		     
	//seems like serial port status info (RTS, CTS,...) is stored in reg(?) 0x0006
	//pcb connection point 11 = GND -> sets bit4 of response
        //pcb connection point  7 = GND -> sets bit6 of response

	//read register:
	ARK3116_RCV_QUIET(0xFE,0xC0,0x0000,0x0006);


	//i do not really know if bit4=CTS and bit6=DSR... was just a quick guess !!
	return  (buf[0] & (1<<4) ? TIOCM_CTS : 0) |
		(buf[0] & (1<<6) ? TIOCM_DSR : 0);
}

static int __init tinkerface_init(void){
	int retval;
	retval = usb_serial_register(&tinkerface_device);
	if (retval)
		return retval;
	retval = usb_register(&tinkerface_driver);
	if (retval)
		usb_serial_deregister(&tinkerface_device);
	return retval;
}

static void __exit tinkerface_exit(void){
	usb_deregister(&tinkerface_driver);
	usb_serial_deregister(&tinkerface_device);
}

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");

module_init(tinkerface_init);
module_exit(tinkerface_exit);
MODULE_LICENSE("GPL");
