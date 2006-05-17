#include "uip/uip.h"

void example_init(void)
{
  uip_listen(HTONS(1234));
}

void example_app(void)
{
  if(uip_newdata() || uip_rexmit())
	  uip_send("ok\n",3);
}
