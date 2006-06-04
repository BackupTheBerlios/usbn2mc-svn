/*
An API for the vscopedevice
Copyright (C) 2006 Benedikt Sauter <sauter@ixbat.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include "vscopedevice.h"


VScope* openVScope()
{
  unsigned char located = 0;
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *device_handle = 0;
 		
  usb_init();
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
  else 
  {
    //usb_set_configuration(vscope_handle,1);
    //usb_claim_interface(vscope_handle,0);
    //usb_set_altinterface(vscope_handle,0);
  
    //return (device_handle);
  }
}


   

int closeVScope(VScope* self)
{
	
}


int sendVScopeCommand(VScope* self,char *command, int length)
{

}

int readVscopeData(VScope* self, char* data)
{
	
}

int readVScopeResults(VScope* self,char *data)
{
	
}


void SetVScopeMode(VScope* self,int state)
{
	
}

void StartVScope(VScope* self)
{

}

void StopVScope(VScope* self)
{
	
}

int GetVScopeState(VScope* self)
{
	
	
}

int GetVScopeMode(VScope* self)
{
	
	
}


int GetVScopeFIFOLoad(VScope* self)
{
	
}


