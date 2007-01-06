#include <stdio.h>

#include "vport.h"

int main()
{
	struct vport * vp_handle;

	printf("libvport Demo\n");
	
	/* open connection to vport */
	vp_handle = vport_open();
	
	if(vp_handle==0)
			fprintf(stderr,"unable to open device\n");

	/* setup serial port */
	vport_serial_configuration(vp_handle,1,SERIAL_DEFAULT);


	/* send data over uart 1 */
	char buf[] = "abc";
	vport_serial_write(vp_handle,1, buf,3);


	vport_close(vp_handle);

	return 0;
}
