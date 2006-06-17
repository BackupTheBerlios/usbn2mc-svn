#include "exampleapp.h"


void example_init(void)
{
  uip_listen(HTONS(1234));
}

void example_app(void)
{
  if(uip_connected())
  {
    uip_send("avr> ",5);
  }
  else if(uip_newdata() || uip_rexmit())
  {
    if(uip_appdata[0]=='h')
      uip_send("ok\navr> ",8);
    else if(uip_appdata[0]=='q')
      uip_close();
    else    
      uip_send("avr> ",5);
  }
  else {
  }
}
