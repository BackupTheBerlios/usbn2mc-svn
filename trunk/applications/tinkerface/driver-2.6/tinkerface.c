/*
 * tinkerface is an own build multi I/O converter 
 * rs232, i2c,p arallel I/O port
 *
 *  Copyright (C) 2006 Benedikt Sauter (sauter@ixbat.de)
 *  
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 *	WITHOUT ANY WARRANTY
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-tinkerfaceeton.c 
 * but has been rewritten to be easy to read and use, as no locks are now
 * needed anymore.
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#include "usb-serial.h" 

static int debug;

/* Define these values to match your devices */
#define USB_TINKERFACE_VENDOR_ID	0xfff0
#define USB_TINKERFACE_PRODUCT_ID	0xfff0

/* table of devices that work with this driver */
static struct usb_device_id tinkerface_table [] = {
	{ USB_DEVICE(USB_TINKERFACE_VENDOR_ID, USB_TINKERFACE_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, tinkerface_table);


/* Get a minor range for your devices from the usb maintainer */
#define USB_TINKERFACE_MINOR_BASE	192

/* Structure to hold all of our device specific stuff */
struct usb_tinkerface {
	struct usb_device *	udev;			/* the usb device for this device */
	struct usb_interface *	interface;		/* the interface for this device */
	unsigned char *		bulk_in_buffer;		/* the buffer to receive data */
	size_t			bulk_in_size;		/* the size of the receive buffer */
	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	__u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
	struct kref		kref;
};
#define to_tinkerface_dev(d) container_of(d, struct usb_tinkerface, kref)

static struct usb_driver tinkerface_driver;

static void tinkerface_delete(struct kref *kref)
{	
	struct usb_tinkerface *dev = to_tinkerface_dev(kref);

	usb_put_dev(dev->udev);
	kfree (dev->bulk_in_buffer);
	kfree (dev);
}

static int tinkerface_open(struct inode *inode, struct file *file)
{
	struct usb_tinkerface *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);

	interface = usb_find_interface(&tinkerface_driver, subminor);
	if (!interface) {
		err ("%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

exit:
	return retval;
}

static int tinkerface_release(struct inode *inode, struct file *file)
{
	struct usb_tinkerface *dev;

	dev = (struct usb_tinkerface *)file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, tinkerface_delete);
	return 0;
}

static ssize_t tinkerface_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	struct usb_tinkerface *dev;
	int retval = 0;
	int bytes_read;

	dev = (struct usb_tinkerface *)file->private_data;
	
	/* do a blocking bulk read to get data from the device */
	retval = usb_bulk_msg(dev->udev,
			      usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			      dev->bulk_in_buffer,
			      min(dev->bulk_in_size, count),
			      &bytes_read, 10000);

	/* if the read was successful, copy the data to userspace */
	if (!retval) {
		if (copy_to_user(buffer, dev->bulk_in_buffer, bytes_read))
			retval = -EFAULT;
		else
			retval = bytes_read;
	}

	return retval;
}

static void tinkerface_write_bulk_callback(struct urb *urb, struct pt_regs *regs)
{
	struct usb_tinkerface *dev;

	dev = (struct usb_tinkerface *)urb->context;

	/* sync/async unlink faults aren't errors */
	if (urb->status && 
	    !(urb->status == -ENOENT || 
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		dbg("%s - nonzero write bulk status received: %d",
		    __FUNCTION__, urb->status);
	}

	/* free up our allocated buffer */
	usb_buffer_free(urb->dev, urb->transfer_buffer_length, 
			urb->transfer_buffer, urb->transfer_dma);
}

static ssize_t tinkerface_write(struct file *file, const char *user_buffer, size_t count, loff_t *ppos)
{
	struct usb_tinkerface *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;

	dev = (struct usb_tinkerface *)file->private_data;

	/* verify that we actually have some data to write */
	if (count == 0)
		goto exit;

	/* create a urb, and a buffer for it, and copy the data to the urb */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

	buf = usb_buffer_alloc(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	if (!buf) {
		retval = -ENOMEM;
		goto error;
	}

	if (copy_from_user(buf, user_buffer, count)) {
		retval = -EFAULT;
		goto error;
	}

	/* initialize the urb properly */
	usb_fill_bulk_urb(urb, dev->udev,
			  usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
			  buf, count, tinkerface_write_bulk_callback, dev);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* send the data out the bulk port */
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		err("%s - failed submitting write urb, error %d", __FUNCTION__, retval);
		goto error;
	}

	/* release our reference to this urb, the USB core will eventually free it entirely */
	usb_free_urb(urb);

exit:
	return count;

error:
	usb_buffer_free(dev->udev, count, buf, urb->transfer_dma);
	usb_free_urb(urb);
	return retval;
}

static struct file_operations tinkerface_fops = {
	.owner =	THIS_MODULE,
	.read =		tinkerface_read,
	.write =	tinkerface_write,
	.open =		tinkerface_open,
	.release =	tinkerface_release,
};

/* 
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 */
static struct usb_class_driver tinkerface_class = {
	.name =		"usb/tinkerface%d",
	.fops =		&tinkerface_fops,
	.mode =		S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
	.minor_base =	USB_TINKERFACE_MINOR_BASE,
};

static int tinkerface_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_tinkerface *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;

	/* allocate memory for our device state and initialize it */
	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		err("Out of memory");
		goto error;
	}
	memset(dev, 0x00, sizeof(*dev));
	kref_init(&dev->kref);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* set up the endpoint information */
	/* use only the first bulk-in and bulk-out endpoints */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->bulk_in_endpointAddr &&
		    ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
					== USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk in endpoint */
			buffer_size = le16_to_cpu(endpoint->wMaxPacketSize);
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->bulk_in_buffer) {
				err("Could not allocate bulk_in_buffer");
				goto error;
			}
		}

		if (!dev->bulk_out_endpointAddr &&
		    ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
					== USB_DIR_OUT) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk out endpoint */
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		err("Could not find both bulk-in and bulk-out endpoints");
		goto error;
	}

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &tinkerface_class);
	if (retval) {
		/* something prevented us from registering this driver */
		err("Not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */
	info("USB Skeleton device now attached to USBSkel-%d", interface->minor);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, tinkerface_delete);
	return retval;
}

static void tinkerface_disconnect(struct usb_interface *interface)
{
	struct usb_tinkerface *dev;
	int minor = interface->minor;

	/* prevent tinkerface_open() from racing tinkerface_disconnect() */
	lock_kernel();

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &tinkerface_class);

	unlock_kernel();

	/* decrement our usage count */
	kref_put(&dev->kref, tinkerface_delete);

	info("USB Skeleton #%d now disconnected", minor);
}

static struct usb_driver tinkerface_driver = {
	.owner =	THIS_MODULE,
	.name =		"tinkerfaceeton",
	.probe =	tinkerface_probe,
	.disconnect =	tinkerface_disconnect,
	.id_table =	tinkerface_table,
};

static int __init usb_tinkerface_init(void)
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register(&tinkerface_driver);
	if (result)
		err("usb_register failed. Error number %d", result);

	return result;
}

static void __exit usb_tinkerface_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&tinkerface_driver);
}

module_init (usb_tinkerface_init);
module_exit (usb_tinkerface_exit);

MODULE_LICENSE("GPL");
