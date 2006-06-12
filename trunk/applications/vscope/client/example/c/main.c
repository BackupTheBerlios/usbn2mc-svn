/*
Demoapplication for libvscopedevice
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
#include "../../lib/vscopedevice.h" 



int main (int argc,char **argv)
{
  VScope *vscope;
  
  vscope = openVScope();

  SetVScopeMode(vscope,MODE_COUNTER);
  StartVScope(vscope);
  int i;
  char buf[20000];

  for(i=0;i<10;i++)
  while(1)
  {
    i = readVScopeData(vscope,buf,20000);
    if(i>0)
      break;
  }
  StopVScope(vscope);


  FILE *datei;
  datei = fopen("vscope.vcd", "w");


  fclose(datei);
  return 0;
}	

