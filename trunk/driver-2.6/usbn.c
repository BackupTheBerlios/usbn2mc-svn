#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <asm/uaccess.h>

MODULE_LICENSE( "GPL" );

#define USB_VENDOR_ID 0x0400
#define USB_DEVICE_ID 0x3412

struct usb_device *dev;

static DECLARE_MUTEX( ulock );

static ssize_t usbn_read( struct file *instanz, char *buffer, size_t count,
        loff_t *ofs )
{
    char pbuf[20];
    __u16 *status = kmalloc( sizeof(__u16), GFP_KERNEL );

    down( &ulock ); // Jetzt nicht disconnecten...
    if( usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_STATUS,
                 USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
                0, 0, status, sizeof(*status), 5*HZ) < 0 ) {
        count = -EIO;
        goto read_out;
    }
    snprintf( pbuf, sizeof(pbuf), "status=%d\n", *status );
    if( strlen(pbuf) < count )
        count = strlen(pbuf);
    count -= copy_to_user(buffer,pbuf,count);
read_out:
    up( &ulock );
    kfree( status );
    return count;
}

static int usbn_open( struct inode *devicefile, struct file *instanz )
{
    return 0;
}


// struct for os global file operations

static struct file_operations usb_fops = {
    .owner = THIS_MODULE,
    .open  = usbn_open,
    .read  = usbn_read,
};

static struct usb_device_id usbid [] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_DEVICE_ID), },
    { }                 /* Terminating entry */
};

struct usb_class_driver class_descr = {
    .name = "usbn",
    .fops = &usb_fops,
    .mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
    .minor_base = 16,
};

static int usbn_probe(struct usb_interface *interface,
    const struct usb_device_id *id)
{
    dev = interface_to_usbdev(interface);
    printk("usbn: 0x%4.4x|0x%4.4x, if=%p\n", dev->descriptor.idVendor,
        dev->descriptor.idProduct, interface );
    if(dev->descriptor.idVendor==USB_VENDOR_ID
        && dev->descriptor.idProduct==USB_DEVICE_ID) {
        if( usb_register_dev( interface, &class_descr ) ) {
            return -EIO;
        }
        printk("got minor= %d\n", interface->minor );
        return 0;
    }
    return -ENODEV;
}

static void usbn_disconnect( struct usb_interface *iface )
{
    down( &ulock ); // Ausstehende Auftraege muessen abgearbeitet sein...
    usb_deregister_dev( iface, &class_descr );
    up( &ulock );
}

static struct usb_driver usbn = {
    .name= "usbn%d",
    .id_table= usbid,
    .probe= usbn_probe,
    .disconnect= usbn_disconnect,
};

static int __init usbn_init(void)
{
    if( usb_register(&usbn) ) {
        printk("usbn: unable to register usb driver\n");
        return -EIO;
    }
    return 0;
}

static void __exit usbn_exit(void)
{
    usb_deregister(&usbn);  
}

module_init(usbn_init);
module_exit(usbn_exit);
/* vim: set ts=4 sw=4 aw ic: */
