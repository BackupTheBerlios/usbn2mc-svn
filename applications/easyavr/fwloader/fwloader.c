#include <stdio.h>
#include <usb.h>

int main(int argc, char **argv)
{
  struct usb_bus *busses;

  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();

  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;

  char buf[64];
  char *ptr = buf;
  int i;

  for(i=0;i<64;i++)
    buf[i]=i;

  unsigned char send_data=0xff;

  for (bus = busses; bus; bus = bus->next)
  {
    struct usb_device *dev;

    for (dev = bus->devices; dev; dev = dev->next)
    {
      if (dev->descriptor.idVendor == 0x0400)
      {
        int i,stat;
        printf("vendor: %i\n",dev->descriptor.idVendor);
        usb_handle = usb_open(dev);
        stat = usb_set_configuration (usb_handle,1);
        printf ("stat:%d from usb_set_config to %x\n",stat,dev->config[0].bConfigurationValue);

        stat=usb_bulk_write(usb_handle,2,ptr,32,100);
        printf("TX stat=%d\n",stat);

        //stat=usb_bulk_read(usb_handle,0x83,buf,2,100);
        //printf("RX stat=%d\n",stat);

        //stat = usb_control_msg(usb_handle,0x40,0x09,0x0200,0x0001,ptr,2,1000);
        //printf("TX stat=%d\n",stat);

        usb_close(usb_handle);
	return 0;
      }
    }
  }
  return 0;
}
