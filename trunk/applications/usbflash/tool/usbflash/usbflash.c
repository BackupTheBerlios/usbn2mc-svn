/* 
 * Benedikt Sauter <sauterbe@rz.fh-augsburg.de> 2006-04-10
 *
 *
 *  Using:
 *  at89flash -r	  // reset 
 *  at89flash -e	  // erase
 *  at89flash -u test.bin // upload 
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h> /* tolower(), toupper(), isalpha() */
#include <stdio.h>
#include <usb.h>


#define FALSE 0
#define TRUE  1
#define BUF 4096

#define RESET 0x00
#define ERASE 0x01
#define UPLOAD 0x02

usb_dev_handle *locate_at89flash(void);
void usbprog_open(usb_dev_handle * usb_handle);

void at89reset(usb_dev_handle * usb_handle);
void at89erase(usb_dev_handle * usb_handle);
void at89upload(usb_dev_handle * usb_handle,char *file);

void show_help(void) {
   printf("\nUsage: usbprog [OPTION]\n"\
          "\t-r  reset controller\n"\
          "\t-e  erase code memory\n"\
          "\t-u  load .bin into code memory\n "\
          "\t-h  Show this help message\n"
          "\t-v  version\n\n");
}


int getoptown(char *argument, char *option) {
   if( argument[0]=='-' && argument[1]==option[0] )
      return 1;
   return 0;
}

int main (int argc,char **argv)
{
  struct usb_dev_handle *usb_handle;
  struct usb_device *usb_device;
  int send_status;
  long erg,port,pin,value;
  unsigned char send_data[4];
  unsigned char receive_data=0;

  usb_init();
  //usb_set_debug(2);
  if ((usb_handle = locate_at89flash())==0) 
  {
    printf("Could not open the at89flasher device\n");
      return (-1);
  }  

   usbprog_open(usb_handle);

   int counter=3;
   char buffer[BUF];
   size_t len=0;

   if(argc == 1 || getoptown(argv[1],"h") == TRUE ) {
      show_help();
      return EXIT_FAILURE;
   }
   else if(getoptown(argv[1],"v") == TRUE) {
      printf("Version 1.0\n");
      return EXIT_SUCCESS;
   }
   else if(argc < 2) {
      show_help();
      return EXIT_FAILURE;
   }
   if(getoptown(argv[1],"r") == TRUE)
      at89reset(usb_handle);
   else if(getoptown(argv[1],"e") == TRUE)
      at89erase(usb_handle);
   else if(getoptown(argv[1],"u") == TRUE)
   {
      if(argc < 3) {
	printf("upload file missing\n");
	return EXIT_FAILURE;
      }
      at89upload(usb_handle,argv[2]);
   }
   else
      show_help();
   return EXIT_SUCCESS;


  usb_close(usb_handle);
 	
}	


void at89reset(usb_dev_handle * usb_handle)
{
  char send[1];
  send[0]=RESET;
  printf("start reset\n");
  usb_bulk_write(usb_handle,2,send,1,1);
  printf("ready\n");
}

void at89erase(usb_dev_handle * usb_handle)
{
  char send[1];
  send[0]=ERASE;
  printf("start erasure\n");
  usb_bulk_write(usb_handle,2,send,1,1);
  printf("ready\n");
}

void at89upload(usb_dev_handle * usb_handle,char *file)
{
  char send[64]; 
  FILE *fd;
  int addr=0;
  int index=4;
  int len;

  printf("start upload\n");
  // header 0x02 startaddrhigh staraddrlow len 
  // split in 60 byte packages
  send[0]=UPLOAD;

  
  
  fd = fopen(file, "r");
  if(!fd) {
    fprintf(stderr, "Unable to open file %s, ignoring.\n", file);
  }
 
  while(!feof(fd))
  { 
    len = 0;
    while(!feof(fd))
    {
      send[index++]=fgetc(fd);
      if(index==63)
	break;
    }
    len = index-4;

    send[1]=(char)(addr>>8);
    send[2]=(char)addr;
    send[3]=(char)len;
    usb_bulk_write(usb_handle,2,send,64,1000);
    addr = addr+len;

    index=4;
  }

  fclose(fd);
 
  printf("ready\n");

}




void usbprog_open(usb_dev_handle * usb_handle)
{
  usb_set_configuration(usb_handle,1);
  usb_claim_interface(usb_handle,0);
  usb_set_altinterface(usb_handle,0);
 
}

usb_dev_handle *locate_at89flash(void) 
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
      }
    }	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

