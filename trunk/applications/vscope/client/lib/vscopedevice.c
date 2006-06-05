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

  VScope *tmp = (VScope*)malloc(sizeof(VScope));
  tmp->vscope_handle=0;
  //usb_set_debug(2);
 		
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
	tmp->vscope_handle = usb_open(dev);
      }
    }	
  }

  if (tmp->vscope_handle==0) return (0);
  else 
  {
    printf("found\n");
    usb_set_configuration(tmp->vscope_handle,1);
    usb_claim_interface(tmp->vscope_handle,0);
    usb_set_altinterface(tmp->vscope_handle,0);
  
    return (tmp);
  }
}


   

int closeVScope(VScope* self)
{
  usb_close(self->vscope_handle);	
}


int sendVScopeCommand(VScope* self,char *command)
{
  return usb_bulk_write(self->vscope_handle,2,command,(int)command[1],1000);
}

int readVScopeData(VScope* self, char* data, int length)
{
  int i;
  i = usb_bulk_read(self->vscope_handle,0x83,data,length,100);	
  printf("%i\n",i);
}

int readVScopeResults(VScope* self,char *data)
{
	
}


void SetVScopeMode(VScope* self,char state)
{
  char command[3] = {CMD_SETMODE,3,state};
  sendVScopeCommand(self,command);
}

void StartVScope(VScope* self)
{
  char command[2] = {CMD_STARTSCOPE,2};
  sendVScopeCommand(self,command);
}

void StopVScope(VScope* self)
{
  char command[2] = {CMD_STOPSCOPE,2};
  sendVScopeCommand(self,command);
}

int GetVScopeState(VScope* self)
{
	
	
}

int GetVScopeMode(VScope* self)
{
	
	
}


int GetVScopeFIFOLoad(VScope* self)
{
  char command[2] = {CMD_GETFIFOLOAD,2};
  sendVScopeCommand(self,command);
}


