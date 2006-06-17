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

#define VALUES 30000
#define LOOPS 1 
#define BYTE unsigned char

int Bit_Test(BYTE val, BYTE bit) {
  BYTE test_val = 0x01;    /* dezimal 1 / binaer 0000 0001 */
  /* Bit an entsprechende Pos. schieben */
  test_val = (test_val << bit);
  /* 0=Bit nicht gesetzt; 1=Bit gesetzt */
  if ((val & test_val) == 0)
    return 0;      /* Nicht gesetzt */
  else
  return 1;      /* gesetzt */
}

int main (int argc,char **argv)
{
  VScope *vscope;
  
  vscope = openVScope();

  SetVScopeMode(vscope,MODE_COUNTER);
  SetVScopeSampleRate(vscope,SAMPLERATE_5US);
  StartVScope(vscope);
  int i,j=0;
  char buf[LOOPS][VALUES];

  while(1)
  {
    i = readVScopeData(vscope,buf[j],VALUES);
    if(i>0)
      j++; 

    if(j>=LOOPS)
      break;
  }
  StopVScope(vscope);

  FILE *datei;
  datei = fopen("vscope.vcd", "w");
  fprintf (datei, "$date\n");
  fprintf (datei, "\tMon Jun 15 17:13:54 1998\n"); 
  fprintf (datei, "$end");
  fprintf (datei, "$version\n");
  fprintf (datei, "Chronologic Simulation VCS version 4.0.3\n");
  fprintf (datei, "$end\n");

  fprintf (datei, "$timescale\n");
  fprintf (datei, "\t1us\n");      
  fprintf (datei, "$end\n");
  fprintf (datei, "$scope module vscope $end\n");
  fprintf (datei, "$var wire       1 !    channel1 $end\n");
  fprintf (datei, "$var wire       1 *    channel2 $end\n");
  fprintf (datei, "$var wire       1 $    channel3 $end\n");
  fprintf (datei, "$var wire       1 (    channel4 $end\n");
  fprintf (datei, "$var wire       1 )    channel5 $end\n");
  fprintf (datei, "$var wire       1 ?    channel6 $end\n");
  fprintf (datei, "$var wire       1 =    channel7 $end\n");
  fprintf (datei, "$var wire       1 +    channel8 $end\n");
  fprintf (datei, "$upscope $end\n");
  fprintf (datei, "$enddefinitions $end\n");

  char sign;

  for(j=0;j<LOOPS;j++)
  {
    for(i=0;i<VALUES;i++)
    {
      sign=buf[j][i];
      fprintf(datei,"#%i\n%i!\n%i*\n%i$\n%i(\n%i)\n%i?\n%i=\n%i+\n",i*2
		    ,Bit_Test(sign, 0)?1:0
		    ,Bit_Test(sign, 1)?1:0
		    ,Bit_Test(sign, 2)?1:0
		    ,Bit_Test(sign, 3)?1:0
		    ,Bit_Test(sign, 4)?1:0
		    ,Bit_Test(sign, 5)?1:0
		    ,Bit_Test(sign, 6)?1:0
		    ,Bit_Test(sign, 7)?1:0);
    }
  }
  fclose(datei);
  return 0;
}	

