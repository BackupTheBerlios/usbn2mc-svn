/* 
 * command for get and set the two ports of vscope device 
 * Benedikt Sauter <sauterbe@rz.fh-augsburg.de> 2006-04-10
 *
 *
 *  Using:
 *  vscope 1	 // get port state in hex format (0x00) every bit is a port state (1-8) 
 *  vscope 1 8     // set port / toggle pin number (1-8)
 */



#include <stdio.h>
#include <usb.h>



usb_dev_handle *locate_vscope(void);
void vscope_open(usb_dev_handle * vscope_handle);

int main (int argc,char **argv)
{
  struct usb_dev_handle *vscope_handle;
  struct usb_device *vscope_device;
  int send_status;
  long erg,port,pin,value;
  unsigned char send_data[4];
  unsigned char receive_data=0;

  usb_init();
  //usb_set_debug(2);
  if ((vscope_handle = locate_vscope())==0) 
  {
    printf("Could not open the vscope device\n");
      return (-1);
  }  

  if((argc-1)==1)
  {
    // get port state
    erg = strtol(argv[1], NULL, 10);
    send_data[0]=(char)erg;

    vscope_open(vscope_handle);

    usb_bulk_write(vscope_handle,2,send_data,2,1);

    usb_bulk_read(vscope_handle,0x83,&receive_data,1,1);	
    printf("%02x\n",receive_data); // get actual port state

    usb_close(vscope_handle);
  }
  else {
    printf("vscope client for i/o hardware\n");
    printf("Using:\n");
    printf("\tvscope 1       // get  channnel 0 - 7\n");
  }
    


 	
}	

void vscope_open(usb_dev_handle * vscope_handle)
{
  int open_status;
  
  open_status = usb_set_configuration(vscope_handle,1);
  //printf("conf_stat=%d\n",open_status);
	
  open_status = usb_claim_interface(vscope_handle,0);
  //printf("claim_stat=%d\n",open_status);
	
  open_status = usb_set_altinterface(vscope_handle,0);
  //printf("alt_stat=%d\n",open_status);
 
}

usb_dev_handle *locate_vscope(void) 
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
	//printf("vscope Device Found @ Address %s \n", dev->filename);
	//printf("vscope Vendor ID 0x0%x\n",dev->descriptor.idVendor);
	//printf("vscope Product ID 0x0%x\n",dev->descriptor.idProduct);
      }
      //else printf("** usb device %s found **\n", dev->filename);			
    }	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

