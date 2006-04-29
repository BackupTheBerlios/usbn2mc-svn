/* 
 * command for get and set the two ports of vlab device 
 * Benedikt Sauter <sauterbe@rz.fh-augsburg.de> 2006-04-10
 *
 *
 *  Using:
 *  vlab 1	 // get port state in hex format (0x00) every bit is a port state (1-8) 
 *  vlab 1 8     // set port / toggle pin number (1-8)
 */



#include <stdio.h>
#include <usb.h>


#define SETPORT 0x01
#define GETPORT 0x02

usb_dev_handle *locate_vlab(void);
void vlab_open(usb_dev_handle * vlab_handle);

int main (int argc,char **argv)
{
  struct usb_dev_handle *vlab_handle;
  struct usb_device *vlab_device;
  int send_status;
  long erg,port,pin,value;
  unsigned char send_data[4];
  unsigned char receive_data=0;

  usb_init();
  //usb_set_debug(2);
  if ((vlab_handle = locate_vlab())==0) 
  {
    printf("Could not open the vlab device\n");
      return (-1);
  }  

  if((argc-1)==1)
  {
    // get port state
    send_data[0]=GETPORT;
    erg = strtol(argv[1], NULL, 10);
    send_data[1]=(char)erg;

    vlab_open(vlab_handle);

    usb_bulk_write(vlab_handle,2,send_data,2,1);

    usb_bulk_read(vlab_handle,0x83,&receive_data,1,1);	
    printf("%02x\n",receive_data); // get actual port state

    usb_close(vlab_handle);
  }
  else if((argc-1)==3) 
  {
    
    //argument portnumber
    port = strtol(argv[1], NULL, 10);
    send_data[1]=(char)port;

    //pin number 
    pin = strtol(argv[2], NULL, 10);

    //pin value 0 or 1 
    value = strtol(argv[3], NULL, 10);

  
    // set port state
    send_data[0]=SETPORT;
    send_data[1]=(char)port;
    send_data[2]=(char)pin;
    send_data[3]=(char)value;

    vlab_open(vlab_handle);

    usb_bulk_write(vlab_handle,2,send_data,4,100);

    usb_bulk_read(vlab_handle,0x83,&receive_data,1,100);	 // get actual port state
    printf("%02x\n",receive_data); // get actual port state

    usb_close(vlab_handle);
  }
  else {
    printf("vlab client for i/o hardware\n");
    printf("Using:\n");
    printf("\tvlab 1       // get port 1 or port 2\n");
    printf("\tvlab 1 2 1    // set port 1 pin 2 value 1\n\n");
  }
    


 	
}	

void vlab_open(usb_dev_handle * vlab_handle)
{
  int open_status;
  
  open_status = usb_set_configuration(vlab_handle,1);
  //printf("conf_stat=%d\n",open_status);
	
  open_status = usb_claim_interface(vlab_handle,0);
  //printf("claim_stat=%d\n",open_status);
	
  open_status = usb_set_altinterface(vlab_handle,0);
  //printf("alt_stat=%d\n",open_status);
 
}

usb_dev_handle *locate_vlab(void) 
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
	//printf("vlab Device Found @ Address %s \n", dev->filename);
	//printf("vlab Vendor ID 0x0%x\n",dev->descriptor.idVendor);
	//printf("vlab Product ID 0x0%x\n",dev->descriptor.idProduct);
      }
      //else printf("** usb device %s found **\n", dev->filename);			
    }	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

