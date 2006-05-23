#include "uip/uip.h"

void example_init(void)
{
  uip_listen(HTONS(1234));
  uip_log("example running");
}


void example_app(void)
{
  uip_log("example app");
 
  if(uip_acked())
  {
      uip_log("->acked");
  }
  if(uip_connected())
  {
      uip_log("->connected");
  }

  if(uip_newdata())
  {
    uip_log("->new data");
  }
/*
  if(uip_newdata() || uip_rexmit()){
    uip_send("ok\n",3);
    uip_log("example ok send");
  }
*/ 
}
