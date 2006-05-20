/* 
 * command for get and set the two ports of usbndemo device 
 * Benedikt Sauter <sauterbe@rz.fh-augsburg.de> 2006-04-10
 *
 *
 *  Using:
 *  usbndemo 1	 // get port state in hex format (0x00) every bit is a port state (1-8) 
 *  usbndemo 1 8     // set port / toggle pin number (1-8)
 */



#include <stdio.h>
#include <usb.h>



usb_dev_handle *locate_usbndemo(void);
void usbndemo_open(usb_dev_handle * usbndemo_handle);

int main (int argc,char **argv)
{
  struct usb_dev_handle *usbndemo_handle;
  struct usb_device *usbndemo_device;
  int send_status,i;
  long erg,port,pin,value;
  unsigned char send_data[1500];
  unsigned char receive_data=0;

  usb_init();
  //usb_set_debug(2);
  if ((usbndemo_handle = locate_usbndemo())==0) 
  {
    printf("Could not open the usbndemo device\n");
      return (-1);
  }  

  usbndemo_open(usbndemo_handle);

  int len=64;
  for(i=0;i<len;i++)
    send_data[i]=0x77;

  //for(i=0;i<1;i++)
  usb_bulk_write(usbndemo_handle,2,send_data,len,1000);

  usb_bulk_read(usbndemo_handle,0x83,&receive_data,120,1000);	
  //printf("%02x\n",receive_data); // get actual port state

  usb_close(usbndemo_handle);
 	
}	

void usbndemo_open(usb_dev_handle * usbndemo_handle)
{
  int open_status;
  
  open_status = usb_set_configuration(usbndemo_handle,1);
  //printf("conf_stat=%d\n",open_status);
	
  open_status = usb_claim_interface(usbndemo_handle,0);
  //printf("claim_stat=%d\n",open_status);
	
  open_status = usb_set_altinterface(usbndemo_handle,0);
  //printf("alt_stat=%d\n",open_status);
 
}

usb_dev_handle *locate_usbndemo(void) 
{
  unsigned char located = 0;
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *device_handle = 0;
 		
  usb_find_busses();
  usb_find_devices();
 
  for (bus = usb_busses; bus; bus = bus->next)
  {
    for (dev = bus->devices; dev; dev = dev->next)	
    {
      if (dev->descriptor.idVendor == 0x0400) 
      {	
	located++;
	device_handle = usb_open(dev);
	//printf("usbndemo Device Found @ Address %s \n", dev->filename);
	//printf("usbndemo Vendor ID 0x0%x\n",dev->descriptor.idVendor);
	//printf("usbndemo Product ID 0x0%x\n",dev->descriptor.idProduct);
      }
      //else printf("** usb device %s found **\n", dev->filename);			
    }	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

