#include "exampleapp.h"

void example_init(void)
{
  uip_listen(HTONS(1234));
  uip_log("example running");
}


void example_app(void)
{
  //uip_log("example app");

  if(uip_connected())
  {
    uip_send("avr> ",5);
  }
  else if(uip_newdata() || uip_rexmit())
  {
    //uip_log("->new data");
    //char html[] = "<html><head><title>BenediktSauter</title></head><body>hallo</body></html>";
    if(uip_appdata[0]=='h')
      uip_send("ok\navr> ",8);
    else    
      uip_send("avr> ",5);
  }
  else {
  }
}
