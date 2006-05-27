#include "uip/uip.h"

void example_init(void)
{
  uip_listen(HTONS(1234));
  uip_log("example running");
}


void example_app(void)
{
  //uip_log("example app");
 
  if(uip_newdata() || uip_rexmit())
  {
    //uip_log("->new data");
    uip_send("ok\n",3);
  }
}
