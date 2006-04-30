#include <stdio.h>
#include <usb.h>
#include <time.h>

clock_t start, end;
double cpu_time_used;

int main(int argc, char **argv)
{
  struct usb_bus *busses;
          
  usb_init();
  usb_find_busses();
  usb_find_devices();
			    
  busses = usb_get_busses();

  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  int c, i, a;
   
  char buf[255];
  char buf2[2];
  char* ptr = buf;
  for (bus = busses; bus; bus = bus->next) 
  {
    struct usb_device *dev;
    
    for (dev = bus->devices; dev; dev = dev->next) 
    {
      /* Check if this device is a printer */
      if (dev->descriptor.idVendor == 0x0400) 
      {
	      int i,stat;	
				printf("vendor: %i\n",dev->descriptor.idVendor);
				usb_handle = usb_open(dev);
	
	

				stat = usb_claim_interface(usb_handle, 0);
				printf ("stat:%d from claim\n",stat);
	
		buf2[0]=0x77;
		buf2[1]=0x88;

		i=usb_bulk_write(usb_handle, 2, buf2, sizeof(buf2), 10);	
		//i=usb_bulk_read(usb_handle, 1, buf2, sizeof(buf2), 10);	

		printf("write: %d\n",i);	
	
		usb_close(usb_handle);
      } 
    }
  } 
  return 0;
}



