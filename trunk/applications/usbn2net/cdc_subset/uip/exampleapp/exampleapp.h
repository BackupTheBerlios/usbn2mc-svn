#ifndef __HTTPD_H__
#define __HTTPD_H__



void example_init(void);

void example_app(void);


struct example_state {
  char *dataptr;
  unsigned int dataleft;
};

#ifndef UIP_APPCALL     
  #define UIP_APPCALL     example_app
#endif

#ifndef UIP_APPSTATE_SIZE 
  #define UIP_APPSTATE_SIZE (sizeof(struct example_state))
#endif

extern struct example_state *es;

#include "../uip/uip.h"

#endif /* __HTTPD_H__ */
                           
